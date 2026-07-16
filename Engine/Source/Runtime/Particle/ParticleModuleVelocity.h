#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;
	class ParticleDistributionVector;

	class ParticleModuleVelocityBase : public ParticleModule
	{
	public:
		ParticleModuleVelocityBase()
			: ParticleModule()
			, bWorldSpace(false)
			, bApplyOwnerScale(false)
		{}

		bool bWorldSpace;
		bool bApplyOwnerScale;

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner);
#endif
	};

// ------------------------------------------------------------------------------------------------------

	class ParticleModuleVelocity : public ParticleModuleVelocityBase
	{
	public:
		ParticleModuleVelocity();

		TRefCountPtr<ParticleDistributionVector> StartVelocity;
		TRefCountPtr<ParticleDistributionFloat> StartVelocityRadial;

		virtual EParticleModule	GetModuleType() const override { return EParticleModule::Velocity; }
		virtual void Spawn(ParticleEmitterInstance* EmitterInstance, int32 Offset, float SpawnTime, BaseParticle* Particle) override;

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

// ------------------------------------------------------------------------------------------------------

//	class ParticleModuleVelocityOverLifetime : public ParticleModuleVelocityBase
//	{
//	public:
//		ParticleModuleVelocityOverLifetime();
//
//		TRefCountPtr<ParticleDistributionVector> VelocityOverLifetime;
//		bool bAbsolute;
//
//		virtual EParticleModule	GetModuleType() const override { return EParticleModule::VelocityOverLifetime; }
//		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* Particle) override;
//		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;
//
//		virtual void Serialize( Archive& Ar ) override;
//
//#if WITH_EDITOR
//		virtual bool Draw(ParticleEmitter* Owner) override;
//#endif
//	};
}