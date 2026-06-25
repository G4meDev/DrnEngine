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

		RandomStream& GetRandomStream( ParticleEmitterInstance* Owner );
	};

}