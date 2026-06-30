#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleModuleSpawnBase : public ParticleModule
	{
	public:
		//uint32 bProcessSpawnRate:1;
		//uint32 bProcessBurstList:1;

		//virtual EModuleType	GetModuleType() const override { return EPMT_Spawn; }

		virtual bool GetSpawnAmount(ParticleEmitterInstance* Owner, float OldLeftover, 
			float DeltaTime, float& Rate)
		{
			//return bProcessSpawnRate;
			return true;
		}
	
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
		//struct FRawDistributionFloat Rate;
		float SpawnRate = 50;

		/** The scalar to apply to the rate. */
		//UPROPERTY(EditAnywhere, Category=Spawn)
		//struct FRawDistributionFloat RateScale;
		//
		///** The method to utilize when burst-emitting particles. */
		//UPROPERTY(EditAnywhere, Category=Burst)
		//TEnumAsByte<EParticleBurstMethod> ParticleBurstMethod;
		//
		///** The array of burst entries. */
		//UPROPERTY(EditAnywhere, export, noclear, Category=Burst)
		//TArray<FParticleBurst> BurstList;
		//
		///** Scale all burst entries by this amount. */
		//UPROPERTY(EditAnywhere, Category=Burst)
		//struct FRawDistributionFloat BurstScale;
		//
		///**	If true, the SpawnRate will be scaled by the global CVar r.EmitterSpawnRateScale */
		//UPROPERTY(EditAnywhere, Category=Spawn)
		//uint32 bApplyGlobalSpawnRateScale : 1;
		//
		///** Initializes the default values for this property */
		//void InitializeDefaults();

		virtual bool GetSpawnAmount(ParticleEmitterInstance* Owner, float OldLeftover, 
			float DeltaTime, float& Rate) override;
		
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::Spawn; }

#if WITH_EDITOR
		inline virtual std::string GetName() const override { return "Spawn"; }
		virtual bool Draw(ParticleEmitterInstance* Owner) override;
#endif

		//virtual float GetMaximumSpawnRate() override;
		//virtual float GetEstimatedSpawnRate() override;
		//virtual int32 GetMaximumBurstCount() override;
	};
}