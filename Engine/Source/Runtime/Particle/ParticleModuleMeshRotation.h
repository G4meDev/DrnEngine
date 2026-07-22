#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;
	class ParticleDistributionVector;

	class ParticleModuleMeshRotation : public ParticleModule
	{
	public:
		ParticleModuleMeshRotation();
	
		TRefCountPtr<ParticleDistributionVector> StartRotation;
		bool bInheritParent;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::MeshRotation; }
		virtual uint32 RequiredBytes() override { return 0; /* size is accounted in emitter. look at bHasMeshRotation */ };

		virtual void CompileModule(ParticleEmitter* Emitter) override;
		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;

		virtual void Serialize( Archive& Ar ) override;
	
#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};

// ----------------------------------------------------------------------------------------------------------

	class ParticleModuleMeshRotationRate : public ParticleModule
	{
	public:
		ParticleModuleMeshRotationRate();
	
		TRefCountPtr<ParticleDistributionVector> StartRotationRate;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::MeshRotationRate; }
		virtual uint32 RequiredBytes() override { return 0; /* size is accounted in emitter. look at bHasMeshRotation */ };

		virtual void CompileModule(ParticleEmitter* Emitter) override;
		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;

		virtual void Serialize( Archive& Ar ) override;
	
#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};

// ----------------------------------------------------------------------------------------------------------

	class ParticleModuleMeshRotationRateOverLifetime : public ParticleModule
	{
	public:
		ParticleModuleMeshRotationRateOverLifetime();
	
		TRefCountPtr<ParticleDistributionVector> RotationRate;
		bool bScaleRotationRate;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::MeshRotationRateOverLifetime; }
		virtual uint32 RequiredBytes() override { return 0; /* size is accounted in emitter. look at bHasMeshRotation */ };

		virtual void CompileModule(ParticleEmitter* Emitter) override;
		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};

// ----------------------------------------------------------------------------------------------------------

	class ParticleModuleMeshRotationRateMultiplyLifetime : public ParticleModule
	{
	public:
		ParticleModuleMeshRotationRateMultiplyLifetime();
	
		TRefCountPtr<ParticleDistributionVector> LifeMultiplier;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::MeshRotationRateMultiplyLifetime; }
		virtual uint32 RequiredBytes() override { return 0; /* size is accounted in emitter. look at bHasMeshRotation */ };

		virtual void CompileModule(ParticleEmitter* Emitter) override;
		virtual void Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase ) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
#endif
	};

}  // namespace Drn