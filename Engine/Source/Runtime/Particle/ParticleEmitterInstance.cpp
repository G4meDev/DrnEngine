#include "DrnPCH.h"
#include "ParticleEmitterInstance.h"

#include "Runtime/Particle/ParticleModuleLocation.h"
#include "Runtime/Particle/ParticleModuleEventGenerator.h"
#include "Runtime/Particle/ParticleModuleEventReceiver.h"

#define MAX_PARTICLE_COUNT 2048

namespace Drn
{
#if WITH_EDITOR
	std::atomic<int32> ParticleStats::ParticleCounter;
#endif

	ParticleEmitterInstance::ParticleEmitterInstance()
		: Emitter(nullptr)
		, Component(nullptr)
		, EmitterType(EEmitterType::Mesh)
		, Location(Vector::ZeroVector)
		, bEnabled(1)
		, bHaltSpawning(0)
		, bIgnoreComponentScale(0)
		, bRenderDataDirty(0)
		, ParticleData(nullptr)
		, ParticleIndices(nullptr)
		, InstanceData(nullptr)
		, InstancePayloadSize(0)
		//, PayloadOffset(0)
		, ParticleSize(0)
		, ParticleStride(0)
		, ActiveParticles(0)
		, ParticleCounter(0)
		, MaxActiveParticles(0)
		, SpawnFraction(0.0f)
		, SecondsSinceCreation(0.0f)
		, EmitterTime(0.0f)
		, LoopCount(0)
		, EmitterDuration(0.0f)
	{
		
	}

	ParticleEmitterInstance::~ParticleEmitterInstance()
	{
		free(ParticleData);
		free(ParticleIndices);
		free(InstanceData);
	}

	void ParticleEmitterInstance::InitParameters( ParticleEmitter* InTemplate, ParticleSystemComponent* InComponent )
	{
		Emitter = InTemplate;
		Component = InComponent;
		SetupEmitterDuration();

		EmitterRandomStream.GenerateNewSeed();
	}

	void ParticleEmitterInstance::Init()
	{
		drn_check(Emitter);

		InstancePayloadSize = Emitter->ReqInstanceBytes;
		InstanceData = (uint8*)(std::realloc(InstanceData, InstancePayloadSize));
		std::memset(InstanceData, 0, InstancePayloadSize);

		for (ParticleModule* ParticleModule : Emitter->Modules)
		{
			drn_check(ParticleModule);
			if (ParticleModule->IsEffectiveModule())
			{
				uint8* PrepInstData = GetModuleInstanceData(ParticleModule);
				ParticleModule->PrepPerInstanceBlock(this, (void*)PrepInstData);
			}
		}

		ParticleSize = Emitter->ParticleSize;
		//ParticleSize += RequiredBytes();
		ParticleSize = Align(ParticleSize, 16);
		ParticleStride = ParticleSize;

		SpawnFraction = 0;
		SecondsSinceCreation = 0;
		EmitterTime = 0;
		ParticleCounter = 0;

		UpdateTransforms();	
		Location = Component->GetWorldLocation();
		OldLocation = Location;

		if (ParticleData == nullptr)
		{
			MaxActiveParticles = 0;
			ActiveParticles = 0;
		}

		Resize(10);

		LoopCount = 0;

		ResetBurstList();

		bRenderDataDirty = true;
		bEmitterIsDone = false;
	}

	World* ParticleEmitterInstance::GetWorld() const 
	{
		return Component->GetWorld();
	}

	void ParticleEmitterInstance::UpdateTransforms()
	{
		Matrix ComponentToWorld = Component != nullptr ?
			Component->GetWorldTransform().ToMatrixNoScale() : Matrix::MatrixIdentity;
		Matrix EmitterToComponent = Transform(Emitter->Origin, Emitter->Rotation);

		if (Emitter->bUseLocalSpace)
		{
			EmitterToSimulation = EmitterToComponent;
			SimulationToWorld = ComponentToWorld;
		}
		else
		{
			EmitterToSimulation = EmitterToComponent * ComponentToWorld;
			SimulationToWorld = Matrix::MatrixIdentity;
		}
	}

	uint32 ParticleEmitterInstance::GetModuleDataOffset( ParticleModule* Module )
	{
		auto It = Emitter->ModuleOffsetMap.find(Module);
		if (It != Emitter->ModuleOffsetMap.end())
		{
			return It->second;
		}
	
		return 0;
	}

	uint8* ParticleEmitterInstance::GetModuleInstanceData( ParticleModule* Module )
	{
		if (InstanceData)
		{
			auto It = Emitter->ModuleInstanceOffsetMap.find(Module);
			if (It != Emitter->ModuleInstanceOffsetMap.end())
			{
				const uint32 Offset = It->second;
				drn_check(Offset < (uint32)InstancePayloadSize);
				return &(InstanceData[Offset]);
			}
		}
		return NULL;
	}

	//uint32 ParticleEmitterInstance::RequiredBytes()
	//{
	//	return sizeof(BaseParticle);
	//}

	bool ParticleEmitterInstance::Resize( int32 NewMaxActiveParticles )
	{
		if (NewMaxActiveParticles < 0 || NewMaxActiveParticles > MAX_PARTICLE_COUNT)
		{
			return false;
		}

		if (NewMaxActiveParticles > MaxActiveParticles)
		{
			ParticleData = (uint8*) realloc(ParticleData, ParticleStride * NewMaxActiveParticles);
			drn_check(ParticleData);

			if (ParticleIndices == nullptr)
			{
				MaxActiveParticles = 0;
			}
			ParticleIndices	= (uint16*) realloc(ParticleIndices, sizeof(uint16) * (NewMaxActiveParticles + 1));

			for (int32 i = MaxActiveParticles; i < NewMaxActiveParticles; i++)
			{
				ParticleIndices[i] = i;
			}

			MaxActiveParticles = NewMaxActiveParticles;
		}

		return true;
	}

	void ParticleEmitterInstance::Tick( float DeltaTime, bool bSuppressSpawning )
	{
		bool bFirstTime = (SecondsSinceCreation > 0.0f) ? false : true;
		Tick_EmitterTimeSetup(DeltaTime);

		if (bEnabled)
		{
			KillParticles();

			ResetParticleParameters(DeltaTime);

			Tick_ModuleUpdate(DeltaTime);
			SpawnFraction = Tick_SpawnParticles(DeltaTime, bSuppressSpawning, bFirstTime);

			// PostUpdate (beams only)
			//Tick_ModulePostUpdate(DeltaTime, LODLevel);

			if (ActiveParticles > 0)
			{
				// Update the orbit data...
				//UpdateOrbitData(DeltaTime);

				UpdateBoundingBox(DeltaTime);

				for (int32 i=0; i<ActiveParticles; i++)
				{
					DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

					Particle.OldLocation = Particle.Location;

					bool bJustSpawned = (Particle.Flags & STATE_Particle_JustSpawned) != 0;
					Particle.Flags &= ~STATE_Particle_JustSpawned;
				}
			}

			//Tick_ModuleFinalUpdate(DeltaTime, LODLevel);

			CheckEmitterFinished();

			// Invalidate the contents of the vertex/index buffer.
			bRenderDataDirty = 1;


		}

		LastDeltaTime = DeltaTime;

#if WITH_EDITOR
		ParticleStats::AddParticleCounter(ActiveParticles);
#endif
	}

	void ParticleEmitterInstance::CheckEmitterFinished()
	{
		if (this->ActiveParticles == 0)
		{
			bool bSpawnFinished = true;
			for (int32 SpawnModIndex = 0; SpawnModIndex < Emitter->SpawningModules.size(); SpawnModIndex++)
			{
				ParticleModuleSpawnBase* SpawnModule = Emitter->SpawningModules[SpawnModIndex];
				if (SpawnModule)
				{
					bSpawnFinished &= SpawnModule->CheckFinished(this);
				}
			}

			if (bSpawnFinished)
			{
				if (HasCompleted() ||
					(Emitter->EmitterDuration == 0
					&& Emitter->EmitterLoops == 0)
					)
				{
					bEmitterIsDone = true;
				}
			}
		}
	}

	void ParticleEmitterInstance::Tick_EmitterTimeSetup( float DeltaTime )
	{
		// Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
		//if (Component->bJustRegistered)
		//{
		//	Location	= Component->GetWorldLocation();
		//	OldLocation	= Location;
		//}
		//else
		//{
		//	// Keep track of location for world space interpolation and other effects.
			OldLocation	= Location;
			Location	= Component->GetWorldLocation();
		//}

		UpdateTransforms();
		SecondsSinceCreation += DeltaTime;

		bool bLooped = false;
		EmitterTime += DeltaTime;
		bLooped = (EmitterDuration > 0.0f) && (EmitterTime >= EmitterDuration);

		if (bLooped)
		{
			LoopCount++;
			ResetBurstList();

			EmitterTime -= EmitterDuration;

			if (Emitter->bDurationRecalcEachLoop)
			{
				SetupEmitterDuration();
			}

//			if (bRequiresLoopNotification == true)
//			{
//				for (int32 ModuleIdx = -3; ModuleIdx < InCurrentLODLevel->Modules.Num(); ModuleIdx++)
//				{
//					int32 ModuleFetchIdx;
//					switch (ModuleIdx)
//					{
//					case -3:	ModuleFetchIdx = INDEX_REQUIREDMODULE;	break;
//					case -2:	ModuleFetchIdx = INDEX_SPAWNMODULE;		break;
//					case -1:	ModuleFetchIdx = INDEX_TYPEDATAMODULE;	break;
//					default:	ModuleFetchIdx = ModuleIdx;				break;
//					}
//
//					UParticleModule* Module = InCurrentLODLevel->GetModuleAtIndex(ModuleFetchIdx);
//					if (Module != NULL)
//					{
//						if (Module->RequiresLoopingNotification() == true)
//						{
//							Module->EmitterLoopingNotify(this);
//						}
//					}
//				}
//			}
		}
	}

	float ParticleEmitterInstance::Tick_SpawnParticles( float DeltaTime, bool bSuppressSpawning, bool bFirstTime )
	{
		if (!bHaltSpawning && !bSuppressSpawning && (EmitterTime >= 0.0f))
		{
			if ((Emitter->EmitterLoops == 0) ||
				(LoopCount < Emitter->EmitterLoops) ||
				(SecondsSinceCreation < (EmitterDuration * Emitter->EmitterLoops)) ||
				bFirstTime)
			{
				bFirstTime = false;
				SpawnFraction = Spawn(DeltaTime);
			}
		}
	
		return SpawnFraction;
	}

	void ParticleEmitterInstance::Tick_ModuleUpdate( float DeltaTime )
	{
		drn_check(Emitter);

		for (int32 ModuleIndex = 0; ModuleIndex < Emitter->UpdateModules.size(); ModuleIndex++)
		{
			ParticleModule* CurrentModule = Emitter->UpdateModules[ModuleIndex];
			if (CurrentModule)
			{
				CurrentModule->Update(this, GetModuleDataOffset(CurrentModule), DeltaTime);
			}
		}
	}

	void ParticleEmitterInstance::ResetParticleParameters( float DeltaTime )
	{
		// Store off any orbit offset values
		//TArray<int32, TInlineAllocator<8>> OrbitOffsets;
		//int32 OrbitCount = LODLevel->OrbitModules.Num();
		//for (int32 OrbitIndex = 0; OrbitIndex < OrbitCount; OrbitIndex++)
		//{
		//	UParticleModuleOrbit* OrbitModule = HighestLODLevel->OrbitModules[OrbitIndex];
		//	if (OrbitModule)
		//	{
		//		uint32* OrbitOffset = SpriteTemplate->ModuleOffsetMap.Find(OrbitModule);
		//		if (OrbitOffset)
		//		{
		//			OrbitOffsets.Add(*OrbitOffset);
		//		}
		//	}
		//}

		for (int32 ParticleIndex = 0; ParticleIndex < ActiveParticles; ParticleIndex++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
			Particle.Velocity = Particle.BaseVelocity;
			Particle.Size = Particle.BaseSize;
			Particle.RotationRate = Particle.BaseRotationRate;
			Particle.Color = Particle.BaseColor;

			bool bJustSpawned = (Particle.Flags & STATE_Particle_JustSpawned) != 0;

			//Don't update position for newly spawned particles. They already have a partial update applied during spawn.
			bool bSkipUpdate = bJustSpawned;

			Particle.RelativeTime += bSkipUpdate ? 0.0f : Particle.OneOverMaxLifetime * DeltaTime;

			//if (CameraPayloadOffset > 0)
			//{
			//	int32 CurrentOffset = CameraPayloadOffset;
			//	const uint8* ParticleBase = (const uint8*)&Particle;
			//	PARTICLE_ELEMENT(FCameraOffsetParticlePayload, CameraOffsetPayload);
			//	CameraOffsetPayload.Offset = CameraOffsetPayload.BaseOffset;
			//}
			//for (int32 OrbitIndex = 0; OrbitIndex < OrbitOffsets.Num(); OrbitIndex++)
			//{
			//	int32 CurrentOffset = OrbitOffsets[OrbitIndex];
			//	const uint8* ParticleBase = (const uint8*)&Particle;
			//	PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
			//	OrbitPayload.PreviousOffset = OrbitPayload.Offset;
			//	OrbitPayload.Offset = OrbitPayload.BaseOffset;
			//	OrbitPayload.RotationRate = OrbitPayload.BaseRotationRate;
			//}
		}
	}

	void ParticleEmitterInstance::ResetBurstList()
	{
		for (int32 SpawnModIndex = 0; SpawnModIndex < Emitter->SpawningModules.size(); SpawnModIndex++)
		{
			ParticleModuleSpawnBase* SpawnModule = Emitter->SpawningModules[SpawnModIndex];
			if (SpawnModule)
			{
				SpawnModule->ResetBurstList(this);
			}
		}
	}

	float ParticleEmitterInstance::Spawn( float DeltaTime )
	{
		drn_check(Emitter);

		float SpawnRate = 0.0f;
		int32 BurstCount = 0;
		float OldLeftover = SpawnFraction;

		for (int32 SpawnModIndex = 0; SpawnModIndex < Emitter->SpawningModules.size(); SpawnModIndex++)
		{
			ParticleModuleSpawnBase* SpawnModule = Emitter->SpawningModules[SpawnModIndex];
			if (SpawnModule)
			{
				float Rate = 0.0f;
				int32 Number = 0;
				SpawnModule->GetSpawnAmount(this, OldLeftover, DeltaTime, Number, Rate);
				Rate = std::max<float>(0.0f, Rate);
				SpawnRate += Rate;

				int32 BurstNumber = 0;
				SpawnModule->GetBurstCount(this, OldLeftover, DeltaTime, BurstNumber);
				BurstCount += BurstNumber;
			}
		}

		if (SpawnRate > 0.0f || BurstCount > 0)
		{
			float SafetyLeftover = OldLeftover;
			// Ensure continuous spawning... lots of fiddling.
			float	NewLeftover = OldLeftover + DeltaTime * SpawnRate;
			int32	Number		= std::floor(NewLeftover);
			float	Increment	= (SpawnRate > 0.0f) ? (1.f / SpawnRate) : 0.0f;
			float	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
			NewLeftover			= NewLeftover - Number;

			bool bProcessSpawn = true;
			int32 NewCount = std::min(ActiveParticles + Number + BurstCount, MAX_PARTICLE_COUNT);

			if (NewCount >= MaxActiveParticles)
			{
				bProcessSpawn = Resize((NewCount + Math::TruncToInt(std::sqrt(std::sqrt((float)NewCount)) + 1)));
			}

			if (bProcessSpawn)
			{
				ParticleEventInstancePayload* EventPayload = nullptr;
				if (Emitter->EventGenerator)
				{
					EventPayload = (ParticleEventInstancePayload*)GetModuleInstanceData(Emitter->EventGenerator);
					if (EventPayload && !EventPayload->bSpawnEventsPresent && !EventPayload->bBurstEventsPresent)
					{
						EventPayload = nullptr;
					}
				}

				const Vector InitialLocation = EmitterToSimulation.Location();
				SpawnParticles( Number, StartTime, Increment, InitialLocation, Vector::ZeroVector, EventPayload );
				SpawnParticles( BurstCount, 0.0f, 0.0f, InitialLocation, Vector::ZeroVector, EventPayload );

				return NewLeftover;
			}
			return SafetyLeftover;
		}

		return SpawnFraction;
	}

	void ParticleEmitterInstance::SpawnParticles( int32 Count, float StartTime, float Increment, const Vector& InitialLocation, const Vector& InitialVelocity, ParticleEventInstancePayload* EventPayload)
	{
		drn_check(ActiveParticles <= MaxActiveParticles);
		drn_check(ActiveParticles + Count <= MaxActiveParticles);
		drn_check(Emitter);

		//Count = FMath::Min<int32>(Count, MaxActiveParticles - ActiveParticles);

		if (EventPayload && EventPayload->bBurstEventsPresent && Count > 0)
		{
			Emitter->EventGenerator->HandleParticleBurst(this, EventPayload, Count);
		}

		float SpawnTime = StartTime;
		float Interp = 1.0f;
		const float InterpIncrement = (Count > 0 && Increment > 0.0f) ? (1.0f / (float)Count) : 0.0f;
		for (int32 i = 0; i < Count; i++)
		{
			uint16 NextFreeIndex = ParticleIndices[ActiveParticles];
			DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * NextFreeIndex);
			const uint32 CurrentParticleIndex = ActiveParticles++;

			PreSpawn(Particle, InitialLocation, InitialVelocity);
			for (int32 ModuleIndex = 0; ModuleIndex < Emitter->SpawnModules.size(); ModuleIndex++)
			{
				ParticleModule* SpawnModule = Emitter->SpawnModules[ModuleIndex];
				if (SpawnModule)
				{
					SpawnModule->Spawn(this, GetModuleDataOffset(SpawnModule), SpawnTime, Particle);
				}
			}
			PostSpawn(Particle, Interp, SpawnTime);

			if (Particle->RelativeTime > 1.0f)
			{
				KillParticle(CurrentParticleIndex);
				continue;
			}

			if (EventPayload)
			{
				if (EventPayload->bSpawnEventsPresent)
				{
					Emitter->EventGenerator->HandleParticleSpawned(this, EventPayload, Particle);
				}
			}

			SpawnTime -= Increment;
			Interp -= InterpIncrement;
		}

	}

	void ParticleEmitterInstance::ForceSpawn( float DeltaTime, int32 InSpawnCount, int32 InBurstCount, Vector& InLocation, Vector& InVelocity )
	{
		int32 SpawnCount = InSpawnCount;
		int32 BurstCount = InBurstCount;
		float SpawnRateDivisor = 0.0f;
		float OldLeftover = 0.0f;

		bool bProcessSpawnRate = true;
		bool bProcessBurstList = true;

		if ((SpawnCount > 0) || (BurstCount > 0))
		{
			int32		Number		= SpawnCount;
			float	Increment	= (SpawnCount > 0) ? (DeltaTime / SpawnCount) : 0;
			float	StartTime	= DeltaTime;
		
			bool bProcessSpawn = true;
			int32 NewCount = ActiveParticles + Number + BurstCount;
			if (NewCount >= MaxActiveParticles)
			{
				bProcessSpawn = Resize(NewCount + Math::TruncToInt(std::sqrt(std::sqrt((float)NewCount)) + 1));
			}

			if (bProcessSpawn == true)
			{
				const bool bUseLocalSpace = Emitter->bUseLocalSpace;
				Vector SpawnLocation = bUseLocalSpace ? Vector::ZeroVector : InLocation;
				Vector SpawnVelocity = bUseLocalSpace ? Vector::ZeroVector : InVelocity;

				SpawnParticles( Number, StartTime, Increment, InLocation, InVelocity, nullptr );
				SpawnParticles( BurstCount, StartTime, 0.0f, InLocation, InVelocity, nullptr );
			}
		}
	}

	void ParticleEmitterInstance::PreSpawn( BaseParticle* Particle, const Vector& InitialLocation, const Vector& InitialVelocity )
	{
		drn_check(Particle);
		drn_check(ParticleSize > 0);

		memset(Particle, 0, ParticleSize);

		Particle->Location = InitialLocation;
		Particle->BaseVelocity = InitialVelocity;
		Particle->Velocity = InitialVelocity;
	}

	void ParticleEmitterInstance::PostSpawn( BaseParticle* Particle, float InterpolationPercentage, float SpawnTime )
	{
		if (!Emitter->bUseLocalSpace)
		{
			if (Vector::DistSquared(OldLocation, Location) > 0.01f)
			{
				Particle->Location = Particle->Location + (OldLocation - Location) * InterpolationPercentage;
			}
		}

		// Offset caused by any velocity
		Particle->OldLocation = Particle->Location;
		Particle->Location    = Particle->Location + Particle->Velocity * SpawnTime;

		// Store a sequence counter.
		Particle->Flags |= ((ParticleCounter++) & STATE_CounterMask);
		Particle->Flags |= STATE_Particle_JustSpawned;
	}

	bool ParticleEmitterInstance::HasCompleted()
	{
		if ((Emitter->EmitterLoops == 0) || 
			(SecondsSinceCreation < (EmitterDuration * Emitter->EmitterLoops)))
		{
			return false;
		}

		if (ActiveParticles > 0)
		{
			return false;
		}

		return true;
	}

	void ParticleEmitterInstance::KillParticles()
	{
		if (ActiveParticles > 0)
		{
			ParticleEventInstancePayload* EventPayload = nullptr;
			if (Emitter->EventGenerator)
			{
				EventPayload = (ParticleEventInstancePayload*)GetModuleInstanceData(Emitter->EventGenerator);
				if (EventPayload && (EventPayload->bDeathEventsPresent == false))
				{
					EventPayload = nullptr;
				}
			}

			bool bFoundCorruptIndices = false;
			// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
			// move them to the 'end' of the active particle list.
			for (int32 i = ActiveParticles - 1; i >= 0; i--)
			{
				const int32	CurrentIndex = ParticleIndices[i];
				drn_check(CurrentIndex < MaxActiveParticles);

				const uint8* ParticleBase = ParticleData + CurrentIndex * ParticleStride;
				BaseParticle& Particle = *((BaseParticle*)ParticleBase);

				if (Particle.RelativeTime > 1.0f)
				{
					if (EventPayload)
					{
						Emitter->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
					}

					// Move it to the 'back' of the list
					ParticleIndices[i] = ParticleIndices[ActiveParticles-1];
					ParticleIndices[ActiveParticles-1]	= CurrentIndex;
					ActiveParticles--;
				}
			}
		}
	}

	void ParticleEmitterInstance::KillParticle( int32 Index )
	{
		if (Index < ActiveParticles)
		{
			ParticleEventInstancePayload* EventPayload = nullptr;
			if (Emitter->EventGenerator)
			{
				EventPayload = (ParticleEventInstancePayload*)GetModuleInstanceData(Emitter->EventGenerator);
				if (EventPayload && (EventPayload->bDeathEventsPresent == false))
				{
					EventPayload = nullptr;
				}
			}

			int32 KillIndex = ParticleIndices[Index];

			if (EventPayload)
			{
				const uint8* ParticleBase	= ParticleData + KillIndex * ParticleStride;
				BaseParticle& Particle		= *((BaseParticle*) ParticleBase);
				Emitter->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
			}

			// Move it to the 'back' of the list
			for (int32 i=Index; i < ActiveParticles - 1; i++)
			{
				ParticleIndices[i] = ParticleIndices[i+1];
			}
			ParticleIndices[ActiveParticles-1] = KillIndex;
			ActiveParticles--;
		}
	}

	void ParticleEmitterInstance::KillParticlesForced( bool bFireEvents )
	{
		ParticleEventInstancePayload* EventPayload = nullptr;
		if (bFireEvents == true)
		{
			if (Emitter->EventGenerator)
			{
				EventPayload = (ParticleEventInstancePayload*)GetModuleInstanceData(Emitter->EventGenerator);
				if (EventPayload && (EventPayload->bDeathEventsPresent == false))
				{
					EventPayload = nullptr;
				}
			}
		}

		for (int32 KillIdx = ActiveParticles - 1; KillIdx >= 0; KillIdx--)
		{
			const int32 CurrentIndex = ParticleIndices[KillIdx];
			if (EventPayload)
			{
				const uint8* ParticleBase = ParticleData + CurrentIndex * ParticleStride;
				BaseParticle& Particle = *((BaseParticle*) ParticleBase);
				Emitter->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
			}
			ParticleIndices[KillIdx] = ParticleIndices[ActiveParticles - 1];
			ParticleIndices[ActiveParticles - 1] = CurrentIndex;
			ActiveParticles--;
		}

		ParticleCounter = 0;
	}

	void ParticleEmitterInstance::SetupEmitterDuration()
	{
		if (Emitter == nullptr)
		{
			return;
		}

		if (Emitter->bEmitterDurationUseRange)
		{
			const float	Rand		= EmitterRandomStream.GetFraction();
			EmitterDuration	= Emitter->EmitterDurationLow + 
				((Emitter->EmitterDuration - Emitter->EmitterDurationLow) * Rand);
		}
		else
		{
			EmitterDuration = Emitter->EmitterDuration;
		}
	}

	void ParticleEmitterInstance::Rewind()
	{
		SecondsSinceCreation = 0;
		EmitterTime = 0;
		LoopCount = 0;
		ParticleCounter = 0;
		bEnabled = 1;
		ResetBurstList();
	}

	void ParticleEmitterInstance::ProcessParticleEvents( float DeltaTime, bool bSuppressSpawning )
	{
		if (Emitter->EventReceiverModules.size() > 0)
		{
			for (int32 EventModIndex = 0; EventModIndex < Emitter->EventReceiverModules.size(); EventModIndex++)
			{
				int32 EventIndex;
				ParticleModuleEventReceiverBase* EventRcvr = Emitter->EventReceiverModules[EventModIndex];
				drn_check(EventRcvr);

				if (EventRcvr->WillProcessParticleEvent(EPET_Spawn) && (Component->SpawnEvents.size() > 0))
				{
					for (EventIndex = 0; EventIndex < Component->SpawnEvents.size(); EventIndex++)
					{
						EventRcvr->ProcessParticleEvent(this, Component->SpawnEvents[EventIndex], DeltaTime);
					}
				}

				if (EventRcvr->WillProcessParticleEvent(EPET_Death) && (Component->DeathEvents.size() > 0))
				{
					for (EventIndex = 0; EventIndex < Component->DeathEvents.size(); EventIndex++)
					{
						EventRcvr->ProcessParticleEvent(this, Component->DeathEvents[EventIndex], DeltaTime);
					}
				}

				if (EventRcvr->WillProcessParticleEvent(EPET_Collision) && (Component->CollisionEvents.size() > 0))
				{
					for (EventIndex = 0; EventIndex < Component->CollisionEvents.size(); EventIndex++)
					{
						EventRcvr->ProcessParticleEvent(this, Component->CollisionEvents[EventIndex], DeltaTime);
					}
				}

				if (EventRcvr->WillProcessParticleEvent(EPET_Burst) && (Component->BurstEvents.size() > 0))
				{
					for (EventIndex = 0; EventIndex < Component->BurstEvents.size(); EventIndex++)
					{
						EventRcvr->ProcessParticleEvent(this, Component->BurstEvents[EventIndex], DeltaTime);
					}
				}

				if (EventRcvr->WillProcessParticleEvent(EPET_Blueprint) && (Component->KismetEvents.size() > 0))
				{
					for (EventIndex = 0; EventIndex < Component->KismetEvents.size(); EventIndex++)
					{
						EventRcvr->ProcessParticleEvent(this, Component->KismetEvents[EventIndex], DeltaTime);
					}
				}
			}
		}
	}

// ----------------------------------------------------------------------------------------------------------------------

	ParticleMeshEmitterInstance::ParticleMeshEmitterInstance()
		//: bHasRotation(false)
		//, MeshRotationOffset(0)
	{
		
	}

	ParticleMeshEmitterInstance::~ParticleMeshEmitterInstance()
	{
		
	}

	void ParticleMeshEmitterInstance::Tick( float DeltaTime, bool bSuppressSpawning )
	{
		if (Emitter->bHasMeshRotation && bEnabled)
		{
			for (int32 i = 0; i < ActiveParticles; i++)
			{
				DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
				MeshRotationPayloadData* PayloadData	= (MeshRotationPayloadData*)((uint8*)&Particle + Emitter->GetMeshRotationOffset());
				PayloadData->RotationRate				= PayloadData->RotationRateBase;

				if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
				{
					PayloadData->Rotation = PayloadData->InitRotation + PayloadData->CurContinuousRotation;
				}
			}
		}

		ParticleEmitterInstance::Tick(DeltaTime, bSuppressSpawning);

		if (Emitter->bHasMeshRotation && bEnabled)
		{
			for (int32 i = 0; i < ActiveParticles; i++)
			{
				DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
				//Particle.Velocity = Particle.Velocity + Vector(0.0, -0.5, 0.0);
	
				MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + Emitter->GetMeshRotationOffset());
				PayloadData->CurContinuousRotation += PayloadData->RotationRate * DeltaTime;
			}
		}

		if (bEnabled && !Component->bWarmingUp)
		{
			for (int32 i = 0; i < ActiveParticles; i++)
			{
				DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
				MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + Emitter->GetMeshRotationOffset());

				Quat ParticleRotation = Emitter->bHasMeshRotation ? Quat(Math::DegreesToRadians(PayloadData->Rotation.GetX()),
					Math::DegreesToRadians(PayloadData->Rotation.GetY()), Math::DegreesToRadians(PayloadData->Rotation.GetZ())) : Quat::Identity;

				//GetWorld()->DrawDebugSphere(SimulationToWorld.TransformPosition(Particle.Location), ParticleRotation, Color::White, 1 - Particle.RelativeTime, 32, 0.01, 0);
				GetWorld()->DrawDebugBox(Box(Particle.Size * -0.5f, Particle.Size * 0.5f), SimulationToWorld * Transform(Particle.Location, ParticleRotation), Color::White, 0.01, 0);
			}
		}
	}

	void ParticleMeshEmitterInstance::PostSpawn( BaseParticle* Particle, float InterpolationPercentage, float SpawnTime )
	{
		ParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);

	}

	//uint32 ParticleMeshEmitterInstance::RequiredBytes()
	//{
	//	uint32 Bytes = ParticleEmitterInstance::RequiredBytes();
	//
	//	if (bHasRotation)
	//	{
	//		MeshRotationOffset = Bytes;
	//		Bytes += sizeof(MeshRotationPayloadData);
	//	}
	//
	//	return Bytes;
	//}

	bool ParticleMeshEmitterInstance::Resize( int32 NewMaxActiveParticles )
	{
		const int32 OldMaxActiveParticles = MaxActiveParticles;

		if (ParticleEmitterInstance::Resize(NewMaxActiveParticles))
		{
			if (Emitter->bHasMeshRotation)
			{
				for (int32 i = OldMaxActiveParticles; i < NewMaxActiveParticles; i++)
				{
					DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
					MeshRotationPayloadData* PayloadData	= (MeshRotationPayloadData*)((uint8*)&Particle + Emitter->GetMeshRotationOffset());
					PayloadData->RotationRateBase			= Vector::ZeroVector;
				}
			}

			return true;
		}

		return false;
	}

	void ParticleMeshEmitterInstance::UpdateBoundingBox( float DeltaTime )
	{
		if (Component && HasActiveParticles())
		{
			 bool bUpdateBox = !Component->bWarmingUp && Component->Template.IsValid() && !Component->Template->bUseFixedBounds;

			Vector Scale = Component->GetWorldScale();

			BoxSphereBounds MeshBound;
			MeshBound = BoxSphereBounds(Box());
			//if (Component->bWarmingUp == false)
			//{	
			//	if (MeshTypeData->Mesh)
			//	{
			//		MeshBound = MeshTypeData->Mesh->GetBounds();
			//	}
			//	else
			//	{
			//		//UE_LOG(LogParticles, Log, TEXT("MeshEmitter with no mesh set?? - %s"), Component->Template ? *(Component->Template->GetPathName()) : TEXT("??????"));
			//		MeshBound = FBoxSphereBounds(FVector(0, 0, 0), FVector(0, 0, 0), 0);
			//	}
			//}
			//else
			//{
			//	// This isn't used anywhere if the bWarmingUp flag is false, but GCC doesn't like it not touched.
			//	FMemory::Memzero(&MeshBound, sizeof(FBoxSphereBounds));
			//}

			const bool bUseLocalSpace = Emitter->bUseLocalSpace;

			const Matrix ComponentToWorld = bUseLocalSpace ? Matrix(Component->GetWorldTransform()) : Matrix::MatrixIdentity;

			Vector	NewLocation;
			float	NewRotation;
			if (bUpdateBox)
			{
				ParticleBoundingBox.Init();
			}

			Vector MinVal(10000000.0f);
			Vector MaxVal(-10000000.0f);
		
			ApplicationMisc::Prefetch(ParticleData, ParticleStride * ParticleIndices[0]);
			ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[0] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);

			for (int32 i=0; i<ActiveParticles; i++)
			{
				DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
				ApplicationMisc::Prefetch(ParticleData, ParticleStride * ParticleIndices[i+1]);
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);

				Particle.OldLocation = Particle.Location;

				bool bJustSpawned = (Particle.Flags & STATE_Particle_JustSpawned) != 0;
				Particle.Flags &= ~STATE_Particle_JustSpawned;

				bool bSkipUpdate = bJustSpawned;

				if ((Particle.Flags & STATE_Particle_Freeze) == 0 && !bSkipUpdate)
				{
					if ((Particle.Flags & STATE_Particle_FreezeTranslation) == 0)
					{
						NewLocation	= Particle.Location + Particle.Velocity * DeltaTime;
					}
					else
					{
						NewLocation = Particle.Location;
					}
					if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
					{
						NewRotation	= Particle.Rotation + DeltaTime * Particle.RotationRate;
					}
					else
					{
						NewRotation = Particle.Rotation;
					}
				}
				else
				{
					// Don't move it...
					NewLocation = Particle.Location;
					NewRotation = Particle.Rotation;
				}

				Vector LocalExtent = MeshBound.GetBox().GetExtent() * Particle.Size * Scale;

				Particle.Rotation = std::fmod(NewRotation, 2.f*(float)XM_PI);
				Particle.Location = NewLocation;

				if (bUpdateBox)
				{	
					Vector PositionForBounds = NewLocation;

					if (bUseLocalSpace)
					{
						// Note: building the bounding box in world space as that gives tighter bounds than transforming a local space AABB into world space
						PositionForBounds = ComponentToWorld.TransformPosition(NewLocation);
					}

					MinVal.SetX( std::min<float>(MinVal.GetX(), PositionForBounds.GetX() - LocalExtent.GetX()) );
					MaxVal.SetX( std::max<float>(MaxVal.GetX(), PositionForBounds.GetX() + LocalExtent.GetX()) );
					MinVal.SetY( std::min<float>(MinVal.GetY(), PositionForBounds.GetY() - LocalExtent.GetY()) );
					MaxVal.SetY( std::max<float>(MaxVal.GetY(), PositionForBounds.GetY() + LocalExtent.GetY()) );
					MinVal.SetZ( std::min<float>(MinVal.GetZ(), PositionForBounds.GetZ() - LocalExtent.GetZ()) );
					MaxVal.SetZ( std::max<float>(MaxVal.GetZ(), PositionForBounds.GetZ() + LocalExtent.GetZ()) );
				}
			}

			if (bUpdateBox)
			{	
				ParticleBoundingBox = Box(MinVal, MaxVal);
			}
		}
	}

        }  // namespace Drn