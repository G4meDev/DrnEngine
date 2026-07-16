#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;

	class ParticleModuleLifetime : public ParticleModule
	{
	public:
		ParticleModuleLifetime();

		TRefCountPtr<ParticleDistributionFloat> Lifetime;

		virtual void Spawn(ParticleEmitterInstance* EmitterInstance, int32 Offset, float SpawnTime, BaseParticle* Particle) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::Lifetime; }

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

}