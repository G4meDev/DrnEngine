#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;
	class ParticleDistributionVector;

	class ParticleModuleAccelerationBase : public ParticleModule
	{
	public:
		ParticleModuleAccelerationBase()
			: ParticleModule()
			, bWorldSpace( false )
		{}

		bool bWorldSpace;

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner );
#endif
	};

// ------------------------------------------------------------------------------------------------------

	class ParticleModuleAccelerationConstant : public ParticleModuleAccelerationBase
	{
	public:
		ParticleModuleAccelerationConstant();

		Vector Acceleration;

		virtual EParticleModule GetModuleType() const override { return EParticleModule::AccelerationConstant; }

		virtual void Spawn( ParticleEmitterInstance* EmitterInstance, float SpawnTime, BaseParticle* Particle ) override;
		virtual void Update(ParticleEmitterInstance* EmitterInstance, int32 Offset, float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};

// ------------------------------------------------------------------------------------------------------

	//class ParticleModuleAccelerationConstant : public ParticleModuleAccelerationBase
	//{
	//public:
	//	ParticleModuleVelocity();
	//
	//	TRefCountPtr<ParticleDistributionVector> StartVelocity;
	//	TRefCountPtr<ParticleDistributionFloat>  StartVelocityRadial;
	//
	//	virtual EParticleModule GetModuleType() const override
	//	{
	//		return EParticleModule::Velocity;
	//	}
	//	virtual void Spawn( ParticleEmitterInstance* EmitterInstance, float SpawnTime, BaseParticle* Particle ) override;
	//
	//	virtual void Serialize( Archive& Ar ) override;
	//
	//#if WITH_EDITOR
	//	virtual bool Draw( ParticleEmitter* Owner ) override;
	//#endif
	//};


}  // namespace Drn