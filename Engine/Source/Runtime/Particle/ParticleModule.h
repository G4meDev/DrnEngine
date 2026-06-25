#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitter;
	class ParticleEmitterInstance;

	enum EModuleType
	{
		EPMT_General,
		EPMT_TypeData,
		EPMT_Beam,
		EPMT_Trail,
		EPMT_Spawn,
		EPMT_Required,
		EPMT_Event,
		EPMT_Light,
		EPMT_SubUV,
		EPMT_MAX,
	};

	class ParticleModule : public RefCountedObject
	{
	public:
		uint8 bSpawnModule:1;
		uint8 bUpdateModule:1;
		uint8 bFinalUpdateModule:1;
		uint8 bUpdateForGPUEmitter:1;

		uint8 bEnabled:1;
		uint8 bEditable:1;

		int32 RandomSeed;

		ParticleModule()
			: bSpawnModule(false)
			, bUpdateModule(false)
			, bFinalUpdateModule(false)
			, bUpdateForGPUEmitter(false)
			, bEnabled(true)
			, bEditable(true)
		{ }

		virtual void CompileModule(ParticleEmitter* Emitter) {};
		virtual void Spawn(ParticleEmitterInstance* EmitterInstance, float SpawnTime, BaseParticle* Particle) {};
		virtual void Update(ParticleEmitterInstance* EmitterInstance, float DeltaTime) {};
		virtual void FinalUpdate(ParticleEmitterInstance* EmitterInstance, float DeltaTime) {};

		virtual EModuleType	GetModuleType() const {	return EPMT_General; }
	};

// ------------------------------------------------------------------------------------------

	class ParticleModuleSpawnBase : public ParticleModule
	{

	public:
		//uint32 bProcessSpawnRate:1;
		//uint32 bProcessBurstList:1;

		virtual EModuleType	GetModuleType() const override { return EPMT_Spawn; }

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

		//virtual float GetMaximumSpawnRate() override;
		//virtual float GetEstimatedSpawnRate() override;
		//virtual int32 GetMaximumBurstCount() override;
	};
}