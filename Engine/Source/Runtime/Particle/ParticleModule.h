#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitter;
	class ParticleEmitterInstance;

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

		//virtual EModuleType	GetModuleType() const {	return EPMT_General; }
		virtual EParticleModule	GetModuleType() const {	return EParticleModule::Spawn; }

		RandomStream& GetRandomStream( ParticleEmitterInstance* Owner );

		inline bool IsEnabled() const { return bEnabled; }
		inline void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

#if WITH_EDITOR
		inline virtual std::string GetName() const { return "Invalid"; }
		virtual bool Draw(ParticleEmitterInstance* Owner) { return false; };
#endif
	};

}