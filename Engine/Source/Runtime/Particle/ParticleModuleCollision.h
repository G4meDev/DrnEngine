#pragma once

namespace Drn
{
	class ParticleDistributionFloat;
	class ParticleDistributionVector;

	enum EParticleCollisionComplete
	{
		EPCC_Kill,
		EPCC_Freeze,
		EPCC_HaltCollisions,
		EPCC_FreezeTranslation,
		EPCC_FreezeRotation,
		EPCC_FreezeMovement,
		EPCC_MAX,
	};

	class ParticleModuleCollision : public ParticleModule
	{
	public:
		ParticleModuleCollision();

		TRefCountPtr<ParticleDistributionVector> DampingFactor;
		TRefCountPtr<ParticleDistributionVector> DampingFactorRotation;

		TRefCountPtr<ParticleDistributionFloat> ParticleMass;
		TRefCountPtr<ParticleDistributionFloat> MaxCollisions;
		TRefCountPtr<ParticleDistributionFloat> DelayAmount;

		EParticleCollisionComplete CollisionCompletionOption;
		std::vector<ECollisionChannel> CollisionTypes;

		bool bApplyPhysics;
		bool bIgnoreSourceActor;
		bool bOnlyVerticalNormalsDecrementCount;

		float MaxCollisionDistance;
		float DirScalar;
		float VerticalFudgeFactor;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::Collision; }

		virtual uint32 RequiredBytes() override;
		virtual uint32 RequiredBytesPerInstance() override;
		virtual uint32 PrepPerInstanceBlock(ParticleEmitterInstance* Owner, void* InstData) override;

		virtual void Spawn(ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* Particle) override;
		virtual void Update(ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

		virtual bool PerformCollisionCheck(ParticleEmitterInstance* Owner, BaseParticle* InParticle, 
			HitResult& Hit, Actor* SourceActor, const Vector& End, const Vector& Start, const Vector& Extent);

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};
}