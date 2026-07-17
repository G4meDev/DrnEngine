#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;

	class ParticleModuleSpawnBase : public ParticleModule
	{
	public:
		ParticleModuleSpawnBase() : ParticleModule()
		{
			bSpawningModule = true;
		}

		virtual void Serialize( Archive& Ar ) override;

		virtual bool GetSpawnAmount(ParticleEmitterInstance* Owner, float OldLeftover, 
			float DeltaTime, int32& Number, float& Rate)
		{
			return true;
		}

		virtual bool GetBurstCount(ParticleEmitterInstance* Owner, float OldLeftover,
			float DeltaTime, int32& Number)
		{
			Number = 0;
			return true;
		}

		virtual bool CheckFinished(ParticleEmitterInstance* Owner)
		{
			return false;
		}

		virtual void ResetBurstList(ParticleEmitterInstance* Owner) {};
	};

	class ParticleModuleSpawn : public ParticleModuleSpawnBase
	{
	public:
		ParticleModuleSpawn();

		TRefCountPtr<ParticleDistributionFloat> SpawnRate;

		std::vector<ParticleBurst> BurstList;

		InterpCurveFloat FloatCurve;
		InterpCurveVector VectorCurve;

		virtual uint32 RequiredBytesPerInstance() override;

		virtual bool GetSpawnAmount(ParticleEmitterInstance* Owner, float OldLeftover, 
			float DeltaTime, int32& Number, float& Rate) override;

		virtual bool GetBurstCount(ParticleEmitterInstance* Owner, float OldLeftover,
			float DeltaTime, int32& Number) override;

		virtual bool CheckFinished(ParticleEmitterInstance* Owner) override;

		virtual void ResetBurstList(ParticleEmitterInstance* Owner) override;

		virtual EParticleModule	GetModuleType() const override { return EParticleModule::Spawn; }

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

// ------------------------------------------------------------------------------------------------------

	class ParticleModuleSpawnPerUnit : public ParticleModuleSpawnBase
	{
	public:
		ParticleModuleSpawnPerUnit();

		float UnitScalar;
		float MovementTolerance;

		TRefCountPtr<ParticleDistributionFloat> SpawnPerUnit;

		float MaxFrameDistance;
		bool bIgnoreSpawnRateWhenMoving;
		bool bIgnoreMovementAlongX;
		bool bIgnoreMovementAlongY;
		bool bIgnoreMovementAlongZ;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::SpawnPerUnit; }

		virtual uint32 RequiredBytesPerInstance() override;

		virtual bool GetSpawnAmount(ParticleEmitterInstance* Owner, float OldLeftover, 
			float DeltaTime, int32& Number, float& Rate) override;

		virtual bool CheckFinished(ParticleEmitterInstance* Owner) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif

	};

}