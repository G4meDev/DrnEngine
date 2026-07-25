#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;

	class ParticleModuleSubuv : public ParticleModule
	{
	public:
		ParticleModuleSubuv();
	
		TRefCountPtr<ParticleDistributionFloat> SubImageIndex;
		bool bUseUpdate;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::Subuv; }
		virtual uint32 RequiredBytes() override { return 0; /* size is accounted in emitter. look at bHasSubuv */ };

		virtual void CompileModule(ParticleEmitter* Emitter) override;
		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;
	
#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};
}
