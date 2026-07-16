#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitter;
	class ParticleEmitterInstance;

	class ParticleModule : public Serializable, public RefCountedObject
	{
	public:
		ParticleModule()
			: bEnabled(true)
			, bValid(true)
			, bSpawningModule(false)
			, bSpawnModule(false)
			, bUpdateModule(false)
		{}

		uint8 bEnabled;
		uint8 bValid;

		uint8 bSpawningModule	:1;
		uint8 bSpawnModule		:1;
		uint8 bUpdateModule		:1;

		virtual void Serialize(Archive& Ar) override;

		virtual void CompileModule(ParticleEmitter* Emitter) {};
		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase) {};
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) {};
		virtual void FinalUpdate(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) {};

		virtual EParticleModule	GetModuleType() const {	return EParticleModule::Spawn; }

		RandomStream& GetRandomStream( ParticleEmitterInstance* Owner );

		inline bool IsEnabled() const { return bEnabled; }
		inline void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

		inline bool IsValid() const { return bValid; }
		inline void SetValid(bool bInValid) { bValid = bInValid; }

		inline bool IsEffectiveModule() const { return bEnabled && bValid; }

		virtual uint32 RequiredBytesPerInstance() { return 0; };
		virtual uint32 RequiredBytes() { return 0; };

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) { return false; };
#endif
	};

}