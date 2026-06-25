#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleModuleLocationBase : public ParticleModule
	{

	};

	class ParticleModuleLocationPrimitiveBase : public ParticleModuleLocationBase
	{
	public:
		ParticleModuleLocationPrimitiveBase()
			: Positive_X(1)
			, Positive_Y(1)
			, Positive_Z(1)
			, Negative_X(1)
			, Negative_Y(1)
			, Negative_Z(1)
			, SurfaceOnly(0)
			, Velocity(0)
		{}

		uint32 Positive_X:1;
		uint32 Positive_Y:1;
		uint32 Positive_Z:1;
		uint32 Negative_X:1;
		uint32 Negative_Y:1;
		uint32 Negative_Z:1;
		uint32 SurfaceOnly:1;
		uint32 Velocity:1;

		//struct FRawDistributionFloat VelocityScale;
		float VelocityScale;

		//struct FRawDistributionVector StartLocation;
		Vector StartLocation = Vector::ZeroVector;

		/** Initializes the default values for this property */
		///void InitializeDefaults();

		virtual void DetermineUnitDirection(ParticleEmitterInstance* Owner, Vector& vUnitDir, struct RandomStream* InRandomStream);
	};

	class ParticleModuleLocationPrimitiveSphere : public ParticleModuleLocationPrimitiveBase
	{
	public:
		//struct FRawDistributionFloat StartRadius;
		float StartRadius = 5.0f;

		virtual void Spawn(ParticleEmitterInstance* Owner, float SpawnTime, BaseParticle* ParticleBase) override;
	};
}