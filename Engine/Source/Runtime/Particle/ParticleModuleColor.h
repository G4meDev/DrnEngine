#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionVector;
	class ParticleDistributionFloat;

	class ParticleModuleColor : public ParticleModule
	{
	public:
		ParticleModuleColor();

		TRefCountPtr<ParticleDistributionVector> StartColor;
		TRefCountPtr<ParticleDistributionFloat> StartAlpha;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::Color; }

		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

// ----------------------------------------------------------------------------------------

	class ParticleModuleColorOverLife : public ParticleModule
	{
	public:
		ParticleModuleColorOverLife();

		TRefCountPtr<ParticleDistributionVector> ColorOverLife;
		TRefCountPtr<ParticleDistributionFloat> AlphaOverLife;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::ColorOverLife; }

		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};
}