#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;
	class ParticleDistributionVector;

	class ParticleModuleLight : public ParticleModule
	{
	public:
		ParticleModuleLight();
	
		TRefCountPtr<ParticleDistributionVector> ColorScaleOverLife;
		TRefCountPtr<ParticleDistributionFloat> BrightnessOverLife;
		TRefCountPtr<ParticleDistributionFloat> RadiusScale;
		bool bUseAbsoluteAttributes;

		virtual EParticleModule GetModuleType() const override { return EParticleModule::Light; }
		virtual uint32 RequiredBytes() override { return 0; /* size is accounted in emitter. look at bHasLight */ };

		virtual void CompileModule(ParticleEmitter* Emitter) override;
		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;
	
#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};
}