#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;
	class ParticleDistributionVector;

	class ParticleModuleLocationBase : public ParticleModule
	{
	public:
		virtual void Serialize( Archive& Ar ) override;

		ParticleModuleLocationBase() : ParticleModule()
		{
			bSpawnModule = true;
		}
	};

	class ParticleModuleLocationPrimitiveBase : public ParticleModuleLocationBase
	{
	public:
		ParticleModuleLocationPrimitiveBase();

		bool Positive_X;
		bool Positive_Y;
		bool Positive_Z;
		bool Negative_X;
		bool Negative_Y;
		bool Negative_Z;
		bool SurfaceOnly;
		bool Velocity;

		TRefCountPtr<ParticleDistributionFloat> VelocityScale;
		TRefCountPtr<ParticleDistributionVector> StartLocation;

		virtual void Serialize( Archive& Ar ) override;

		virtual void DetermineUnitDirection(ParticleEmitterInstance* Owner, Vector& vUnitDir, struct RandomStream* InRandomStream);

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

	class ParticleModuleLocationPrimitiveSphere : public ParticleModuleLocationPrimitiveBase
	{
	public:
		ParticleModuleLocationPrimitiveSphere();

		TRefCountPtr<ParticleDistributionFloat> StartRadius;

		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase) override;

		virtual EParticleModule	GetModuleType() const override { return EParticleModule::LocationSphere; }

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};
}