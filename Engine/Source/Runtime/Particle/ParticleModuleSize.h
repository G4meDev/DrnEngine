#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionVector;

	class ParticleModuleSize : public ParticleModule
	{
	public:
		ParticleModuleSize();

		TRefCountPtr<ParticleDistributionVector> StartSize;

		virtual void Spawn(ParticleEmitterInstance* EmitterInstance, int32 Offset, float SpawnTime, BaseParticle* Particle) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::Size; }

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

// ----------------------------------------------------------------------------------------------

	class ParticleModuleSizeScale : public ParticleModule
	{
	public:
		ParticleModuleSizeScale();

		TRefCountPtr<ParticleDistributionVector> SizeScale;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::SizeScale; }

		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

// ----------------------------------------------------------------------------------------------

	class ParticleModuleSizeByLife : public ParticleModule
	{
	public:
		ParticleModuleSizeByLife();

		TRefCountPtr<ParticleDistributionVector> LifeMultiplier;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::SizeByLife; }

		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};


}