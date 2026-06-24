#include "DrnPCH.h"
#include "ParticleEmitterInstance.h"

#define MAX_PARTICLE_COUNT 2048

namespace Drn
{
	ParticleEmitterInstance::ParticleEmitterInstance()
		: Emitter(nullptr)
		, Compponent(nullptr)
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
		//, InstanceData(NULL)
		//, InstancePayloadSize(0)
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
		//free(InstanceData);
	}

	void ParticleEmitterInstance::InitParameters( ParticleEmitter* InTemplate, ParticleSystemComponent* InComponent )
	{
		Emitter = InTemplate;
		Compponent = InComponent;
		//SetupEmitterDuration();
	}

	void ParticleEmitterInstance::Init()
	{
		

		ParticleSize = RequiredBytes();
		ParticleSize = Align(ParticleSize, 16);
		ParticleStride = ParticleSize;

		SpawnFraction = 0;
		SecondsSinceCreation = 0;
		EmitterTime = 0;
		ParticleCounter = 0;

		//UpdateTransforms();	
		Location = Compponent->GetWorldLocation();
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
		return Compponent->GetWorld();
	}

	void ParticleEmitterInstance::UpdateTransforms()
	{
		
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
		if (bEnabled)
		{
			KillParticles();

		}
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

// ----------------------------------------------------------------------------------------------------------------------

	ParticleMeshEmitterInstance::ParticleMeshEmitterInstance()
		: bHasRotation(false)
		, MeshRotationOffset(0)
	{
		RandStream.Initalize(std::rand());
	}

	ParticleMeshEmitterInstance::~ParticleMeshEmitterInstance()
	{
		
	}

	void ParticleMeshEmitterInstance::Tick( float DeltaTime )
	{
		ParticleEmitterInstance::Tick(DeltaTime);

		int32 SpawnRate = 45;
		float SpawnInterval = 1.0f / SpawnRate;

		int32 SpawnCount = 0;
		SpawnFraction += DeltaTime;
		while (SpawnFraction > SpawnInterval)
		{
			SpawnFraction -= SpawnInterval;
			SpawnCount++;
		}

		int32 OldActiveParticles = ActiveParticles;
		ActiveParticles = std::min(OldActiveParticles + SpawnCount, MAX_PARTICLE_COUNT);
		Resize(ActiveParticles);

		for (int32 i = OldActiveParticles; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			Particle.OldLocation = Particle.Location = RandStream.GetUnitVector() * RandStream.FRandRange(-5, 5);
			Particle.OneOverMaxLifetime = 1.0f / RandStream.FRandRange(0.3f, 0.7f);
			Particle.RelativeTime = 0.0;
			Particle.BaseVelocity = Vector::ZeroVector;
			Particle.BaseRotationRate = 0.0f;
			Particle.Velocity = RandStream.GetUnitVector() * RandStream.FRandRange(3, 5);
			Particle.Rotation = 0.0f;
		}

		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			Particle.RelativeTime += DeltaTime * Particle.OneOverMaxLifetime;
			Particle.Velocity = Particle.Velocity + Vector(0.0, -0.5, 0.0);
			Particle.Location = Particle.Location + Particle.Velocity * DeltaTime;

			GetWorld()->DrawDebugSphere(Compponent->GetWorldTransform().TransformPosition(Particle.Location), Quat::Identity, Color::White, 1 - Particle.RelativeTime, 32, 0.01, 0);
		}
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