#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleModuleSpawnBase : public ParticleModule
	{
	public:
		ParticleModuleSpawnBase() : ParticleModule()
		{
			bSpawningModule = true;
		}
		//uint32 bProcessSpawnRate:1;
		//uint32 bProcessBurstList:1;

		//virtual EModuleType	GetModuleType() const override { return EPMT_Spawn; }

		virtual bool GetSpawnAmount(ParticleEmitterInstance* Owner, float OldLeftover, 
			float DeltaTime, int32& Number, float& Rate)
		{
			//return bProcessSpawnRate;
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

		//virtual bool GetBurstCount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover, 
		//	float DeltaTime, int32& Number)
		//{
		//	Number = 0;
		//	return bProcessBurstList;
		//}

		//virtual float GetEstimatedSpawnRate() { return 0.0f; }
		//virtual int32 GetMaximumBurstCount() { return 0; }
	};

	class ParticleModuleSpawn : public ParticleModuleSpawnBase
	{
	public:
		ParticleModuleSpawn() : ParticleModuleSpawnBase()
		{}

		//struct FRawDistributionFloat Rate;
		float SpawnRate = 50;

		///** The method to utilize when burst-emitting particles. */
		//UPROPERTY(EditAnywhere, Category=Burst)
		//TEnumAsByte<EParticleBurstMethod> ParticleBurstMethod;

		std::vector<ParticleBurst> BurstList;

		///** Initializes the default values for this property */
		//void InitializeDefaults();

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

		//virtual float GetMaximumSpawnRate() override;
		//virtual float GetEstimatedSpawnRate() override;
		//virtual int32 GetMaximumBurstCount() override;
	};

// ------------------------------------------------------------------------------------------------------

	class ParticleModuleSpawnPerUnit : public ParticleModuleSpawnBase
	{
	public:
		ParticleModuleSpawnPerUnit() : ParticleModuleSpawnBase()
			, UnitScalar(5.0f)
			, MovementTolerance(0.1f)
			, SpawnPerUnit(1.0f)
			, MaxFrameDistance(0.0f)
			, bIgnoreSpawnRateWhenMoving(false)
			, bIgnoreMovementAlongX(false)
			, bIgnoreMovementAlongY(false)
			, bIgnoreMovementAlongZ(false)
		{}

		float UnitScalar;
		float MovementTolerance;

		//FRawDistributionFloat SpawnPerUnit;
		float SpawnPerUnit;

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