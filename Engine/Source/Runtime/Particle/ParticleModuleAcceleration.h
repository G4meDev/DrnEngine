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

		virtual void Spawn( ParticleEmitterInstance* EmitterInstance, int32 Offset, float SpawnTime, BaseParticle* Particle ) override;
		virtual void Update(ParticleEmitterInstance* EmitterInstance, int32 Offset, float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};

// ------------------------------------------------------------------------------------------------------

	class ParticleModuleAcceleration : public ParticleModuleAccelerationBase
	{
	public:
		ParticleModuleAcceleration();
	
		TRefCountPtr<ParticleDistributionVector> Acceleration;
		bool bApplyOwnerScale;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::Acceleration; }
		virtual uint32 RequiredBytes() override { return sizeof(Vector); };

		virtual void Spawn( ParticleEmitterInstance* EmitterInstance, int32 Offset, float SpawnTime, BaseParticle* Particle ) override;
		virtual void Update(ParticleEmitterInstance* EmitterInstance, int32 Offset, float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;
	
#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};


}  // namespace Drn