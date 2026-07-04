#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleModuleLocationBase : public ParticleModule
	{
	public:
		ParticleModuleLocationBase() : ParticleModule()
		{
			bSpawnModule = true;
		}
	};

	class ParticleModuleLocationPrimitiveBase : public ParticleModuleLocationBase
	{
	public:
		ParticleModuleLocationPrimitiveBase() : ParticleModuleLocationBase()
			, Positive_X(1)
			, Positive_Y(1)
			, Positive_Z(1)
			, Negative_X(1)
			, Negative_Y(1)
			, Negative_Z(1)
			, SurfaceOnly(0)
			, Velocity(0)
			, VelocityScale(1)
		{}

		bool Positive_X;
		bool Positive_Y;
		bool Positive_Z;
		bool Negative_X;
		bool Negative_Y;
		bool Negative_Z;
		bool SurfaceOnly;
		bool Velocity;

		//struct FRawDistributionFloat VelocityScale;
		float VelocityScale;

		//struct FRawDistributionVector StartLocation;
		Vector StartLocation = Vector::ZeroVector;

		/** Initializes the default values for this property */
		///void InitializeDefaults();

		virtual void Serialize( Archive& Ar ) override;

		virtual void DetermineUnitDirection(ParticleEmitterInstance* Owner, Vector& vUnitDir, struct RandomStream* InRandomStream);

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

	class ParticleModuleLocationPrimitiveSphere : public ParticleModuleLocationPrimitiveBase
	{
	public:
		ParticleModuleLocationPrimitiveSphere() : ParticleModuleLocationPrimitiveBase()
		{}

		//struct FRawDistributionFloat StartRadius;
		float StartRadius = 5.0f;

		virtual void Spawn(ParticleEmitterInstance* Owner, float SpawnTime, BaseParticle* ParticleBase) override;

		virtual EParticleModule	GetModuleType() const override { return EParticleModule::LocationSphere; }

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};
}