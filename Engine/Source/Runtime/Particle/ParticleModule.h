#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitter;
	class ParticleEmitterInstance;

	class ParticleModule : public Serializable, public RefCountedObject
	{
	public:
		uint8 bEnabled;

		ParticleModule()
			: bEnabled(true)
		{}

		virtual void Serialize(Archive& Ar) override;

		virtual void CompileModule(ParticleEmitter* Emitter) {};
		virtual void Spawn(ParticleEmitterInstance* EmitterInstance, float SpawnTime, BaseParticle* Particle) {};
		virtual void Update(ParticleEmitterInstance* EmitterInstance, float DeltaTime) {};
		virtual void FinalUpdate(ParticleEmitterInstance* EmitterInstance, float DeltaTime) {};

		virtual EParticleModule	GetModuleType() const {	return EParticleModule::Spawn; }

		RandomStream& GetRandomStream( ParticleEmitterInstance* Owner );

		inline bool IsEnabled() const { return bEnabled; }
		inline void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) { return false; };
#endif
	};

}