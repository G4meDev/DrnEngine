#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;

	class ParticleModuleRotation : public ParticleModule
	{
	public:
		ParticleModuleRotation();
	
		TRefCountPtr<ParticleDistributionFloat> StartRotation;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::Rotation; }
		virtual void Serialize( Archive& Ar ) override;

		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;
	
#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};

	class ParticleModuleRotationRate : public ParticleModule
	{
	public:
		ParticleModuleRotationRate();
	
		TRefCountPtr<ParticleDistributionFloat> StartRotationRate;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::RotationRate; }
		virtual void Serialize( Archive& Ar ) override;

		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;
	
#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};
}