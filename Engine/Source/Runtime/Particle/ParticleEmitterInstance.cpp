#include "DrnPCH.h"
#include "ParticleEmitterInstance.h"

#include "Runtime/Particle/ParticleModuleLocation.h"

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
		, bKillOnDeactivate(0)
		, bKillOnCompleted(0)
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
		//SetupEmitterDuration();

		EmitterRandomStream.GenerateNewSeed();
	}

	void ParticleEmitterInstance::Init()
	{
		drn_check(InstanceData == nullptr);

		InstancePayloadSize = Emitter->ReqInstanceBytes;
		InstanceData = (uint8*)(std::realloc(InstanceData, InstancePayloadSize));
		std::memset(InstanceData, 0, InstancePayloadSize);

		ParticleSize = RequiredBytes();
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
		Matrix EmitterToComponent = Transform(Origin, Rotation);

		if (bUseLocalSpace)
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

	uint32 ParticleEmitterInstance::RequiredBytes()
	{
		return sizeof(BaseParticle);
	}

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

	void ParticleEmitterInstance::Tick( float DeltaTime )
	{
		bool bFirstTime = (SecondsSinceCreation > 0.0f) ? false : true;
		Tick_EmitterTimeSetup(DeltaTime);

		if (bEnabled)
		{
			KillParticles();

			ResetParticleParameters(DeltaTime);

			Tick_ModuleUpdate(DeltaTime);
			SpawnFraction = Tick_SpawnParticles(DeltaTime, bFirstTime);

			// PostUpdate (beams only)
			//Tick_ModulePostUpdate(DeltaTime, LODLevel);

			if (ActiveParticles > 0)
			{
				// Update the orbit data...
				//UpdateOrbitData(DeltaTime);
				// Calculate bounding box and simulate velocity.
				//UpdateBoundingBox(DeltaTime);

				for (int32 i=0; i<ActiveParticles; i++)
				{
					DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

					Particle.OldLocation = Particle.Location;

					bool bJustSpawned = (Particle.Flags & STATE_Particle_JustSpawned) != 0;
					Particle.Flags &= ~STATE_Particle_JustSpawned;
				}
			}

			//Tick_ModuleFinalUpdate(DeltaTime, LODLevel);

			//CheckEmitterFinished();

			// Invalidate the contents of the vertex/index buffer.
			bRenderDataDirty = 1;


		}

		EmitterTime += DeltaTime;
		LastDeltaTime = DeltaTime;

#if WITH_EDITOR
		ParticleStats::AddParticleCounter(ActiveParticles);
#endif
	}

	float ParticleEmitterInstance::Tick_EmitterTimeSetup( float DeltaTime )
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

//		bool bLooped = false;
//		if (InCurrentLODLevel->RequiredModule->bUseLegacyEmitterTime == false)
//		{
//			EmitterTime += DeltaTime;
//			bLooped = (EmitterDuration > 0.0f) && (EmitterTime >= EmitterDuration);
//		}
//		else
//		{
//			EmitterTime = SecondsSinceCreation;
//			if (EmitterDuration > KINDA_SMALL_NUMBER)
//			{
//				EmitterTime = FMath::Fmod(SecondsSinceCreation, EmitterDuration);
//				bLooped = ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration);
//			}
//		}
//
//		// Get the emitter delay time
//		float EmitterDelay = CurrentDelay;
//
//		// Determine if the emitter has looped
//		if (bLooped)
//		{
//			LoopCount++;
//			ResetBurstList();
//	#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
//			// Reset the event count each loop...
//			if (EventCount > MaxEventCount)
//			{
//				MaxEventCount = EventCount;
//			}
//			EventCount = 0;
//	#endif	//#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
//
//			if (InCurrentLODLevel->RequiredModule->bUseLegacyEmitterTime == false)
//			{
//				EmitterTime -= EmitterDuration;
//			}
//
//			if ((InCurrentLODLevel->RequiredModule->bDurationRecalcEachLoop == true)
//				|| ((InCurrentLODLevel->RequiredModule->bDelayFirstLoopOnly == true) && (LoopCount == 1))
//				)
//			{
//				SetupEmitterDuration();
//			}
//
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
//		}
//
//		// Don't delay unless required
//		if ((InCurrentLODLevel->RequiredModule->bDelayFirstLoopOnly == true) && (LoopCount > 0))
//		{
//			EmitterDelay = 0;
//		}
//
//		// 'Reset' the emitter time so that the modules function correctly
//		EmitterTime -= EmitterDelay;

		float EmitterDelay = 0.0f;
		return EmitterDelay;
	}

	float ParticleEmitterInstance::Tick_SpawnParticles( float DeltaTime, bool bFirstTime )
	{
		if (!bHaltSpawning && (EmitterTime >= 0.0f))
		{
			// If emitter is not done - spawn at current rate.
			// If EmitterLoops is 0, then we loop forever, so always spawn.
			//if ((InCurrentLODLevel->RequiredModule->EmitterLoops == 0) ||
			//	(LoopCount < InCurrentLODLevel->RequiredModule->EmitterLoops) ||
			//	(SecondsSinceCreation < (EmitterDuration * InCurrentLODLevel->RequiredModule->EmitterLoops)) ||
			//	bFirstTime)
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
				CurrentModule->Update(this, DeltaTime);
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
			//Particle.Velocity = Particle.BaseVelocity;
			//Particle.Size = Particle.BaseSize;
			//Particle.RotationRate = Particle.BaseRotationRate;
			//Particle.Color = Particle.BaseColor;

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

	float ParticleEmitterInstance::Spawn( float DeltaTime )
	{
		drn_check(Emitter);

		float SpawnRate = 0.0f;
		float OldLeftover = SpawnFraction;

		for (int32 SpawnModIndex = 0; SpawnModIndex < Emitter->SpawningModules.size(); SpawnModIndex++)
		{
			ParticleModuleSpawnBase* SpawnModule = Emitter->SpawningModules[SpawnModIndex];
			if (SpawnModule)
			{
				float Rate = 0.0f;
				int32 Number = 0;
				const int32 Offset = 0;
				SpawnModule->GetSpawnAmount(this, Offset, OldLeftover, DeltaTime, Number, Rate);
				Rate = std::max<float>(0.0f, Rate);
				SpawnRate += Rate;
			}
		}

		if (SpawnRate > 0.f)
		{
			float SafetyLeftover = OldLeftover;
			// Ensure continuous spawning... lots of fiddling.
			float	NewLeftover = OldLeftover + DeltaTime * SpawnRate;
			int32	Number		= std::floor(NewLeftover);
			float	Increment	= (SpawnRate > 0.0f) ? (1.f / SpawnRate) : 0.0f;
			float	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
			NewLeftover			= NewLeftover - Number;

			// Handle growing arrays.
			bool bProcessSpawn = true;
			int32 NewCount = std::min(ActiveParticles + Number, MAX_PARTICLE_COUNT);

			if (NewCount >= MaxActiveParticles)
			{
				bProcessSpawn = Resize((NewCount + Math::TruncToInt(std::sqrt(std::sqrt((float)NewCount)) + 1)));
			}

			if (bProcessSpawn == true)
			{
				const Vector InitialLocation = EmitterToSimulation.Location();
				SpawnParticles( Number, StartTime, Increment, InitialLocation, Vector::ZeroVector );

				return NewLeftover;
			}
			return SafetyLeftover;
		}

		return SpawnFraction;
	}

	void ParticleEmitterInstance::SpawnParticles( int32 Count, float StartTime, float Increment, const Vector& InitialLocation, const Vector& InitialVelocity)
	{
		drn_check(ActiveParticles <= MaxActiveParticles);
		drn_check(ActiveParticles + Count <= MaxActiveParticles);
		drn_check(Emitter);

		//Count = FMath::Min<int32>(Count, MaxActiveParticles - ActiveParticles);
	
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
					SpawnModule->Spawn(this, SpawnTime, Particle);
				}
			}
			PostSpawn(Particle, Interp, SpawnTime);

			if (Particle->RelativeTime > 1.0f)
			{
				KillParticle(CurrentParticleIndex);
				continue;
			}

			SpawnTime -= Increment;
			Interp -= InterpIncrement;
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
		//if (LODLevel->RequiredModule->bUseLocalSpace == false)
		//{
		//	if (FVector::DistSquared(OldLocation, Location) > 1.f)
		//	{
		//		Particle->Location += InterpolationPercentage * (OldLocation - Location);	
		//	}
		//}

		// Offset caused by any velocity
		Particle->OldLocation = Particle->Location;
		Particle->Location    = Particle->Location + Particle->Velocity * SpawnTime;

		// Store a sequence counter.
		Particle->Flags |= ((ParticleCounter++) & STATE_CounterMask);
		Particle->Flags |= STATE_Particle_JustSpawned;
	}

	void ParticleEmitterInstance::KillParticles()
	{
		if (ActiveParticles > 0)
		{
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
					//if (EventPayload)
					//{
					//	LODLevel->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
					//}

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
			int32 KillIndex = ParticleIndices[Index];

			// Move it to the 'back' of the list
			for (int32 i=Index; i < ActiveParticles - 1; i++)
			{
				ParticleIndices[i] = ParticleIndices[i+1];
			}
			ParticleIndices[ActiveParticles-1] = KillIndex;
			ActiveParticles--;
		}
	}

// ----------------------------------------------------------------------------------------------------------------------

	ParticleMeshEmitterInstance::ParticleMeshEmitterInstance()
		: bHasRotation(false)
		, MeshRotationOffset(0)
	{
		
	}

	ParticleMeshEmitterInstance::~ParticleMeshEmitterInstance()
	{
		
	}

	void ParticleMeshEmitterInstance::Tick( float DeltaTime )
	{
		ParticleEmitterInstance::Tick(DeltaTime);

		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			Particle.Velocity = Particle.Velocity + Vector(0.0, -0.5, 0.0);
			Particle.Location = Particle.Location + Particle.Velocity * DeltaTime;

			GetWorld()->DrawDebugSphere(SimulationToWorld.TransformPosition(Particle.Location), Quat::Identity, Color::White, 1 - Particle.RelativeTime, 32, 0.01, 0);
		}
	}

	void ParticleMeshEmitterInstance::PostSpawn( BaseParticle* Particle, float InterpolationPercentage, float SpawnTime )
	{
		ParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);

		//Particle->Location = EmitterToSimulation.TransformPosition(RandStream.GetUnitVector() * RandStream.FRandRange(-5, 5));
		Particle->OneOverMaxLifetime = 1.0f / EmitterRandomStream.FRandRange(0.3f, 0.7f);
		//Particle->Velocity = RandStream.GetUnitVector() * RandStream.FRandRange(3, 5);
	}

	uint32 ParticleMeshEmitterInstance::RequiredBytes()
	{
		uint32 Bytes = ParticleEmitterInstance::RequiredBytes();

		if (bHasRotation)
		{
			MeshRotationOffset = Bytes;
			Bytes += sizeof(MeshRotationPayloadData);
		}

		return Bytes;
	}

	bool ParticleMeshEmitterInstance::Resize( int32 NewMaxActiveParticles )
	{
		const int32 OldMaxActiveParticles = MaxActiveParticles;

		if (ParticleEmitterInstance::Resize(NewMaxActiveParticles))
		{
			if (bHasRotation)
			{
				for (int32 i = OldMaxActiveParticles; i < NewMaxActiveParticles; i++)
				{
					DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
					MeshRotationPayloadData* PayloadData	= (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
					PayloadData->RotationRateBase			= Vector::ZeroVector;
				}
			}

			return true;
		}

		return false;
	}

        }  // namespace Drn