#include "DrnPCH.h"
#include "ParticleModuleRotation.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleRotation::ParticleModuleRotation()
		: ParticleModule()
		, StartRotation(new ParticleDistributionFloatConstant(0.0f))
	{
		bSpawnModule = true;
	}

	void ParticleModuleRotation::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			StartRotation = ParticleDistributionFloat::Create(Ar);
		}
		else
		{
			StartRotation->Serialize(Ar);
		}
	}

	void ParticleModuleRotation::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			Particle.Rotation += (XM_PI/180.f) * 360.0f * StartRotation->GetValue(Owner->EmitterTime, Owner);
		}
	}

// --------------------------------------------------------------------------------------------

	ParticleModuleRotationRate::ParticleModuleRotationRate()
		: ParticleModule()
		, StartRotationRate(new ParticleDistributionFloatConstant(0.0f))
	{
		bSpawnModule = true;
	}

	void ParticleModuleRotationRate::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			StartRotationRate = ParticleDistributionFloat::Create(Ar);
		}
		else
		{
			StartRotationRate->Serialize(Ar);
		}
	}

	void ParticleModuleRotationRate::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			float StartRotRate = (XM_PI/180.f) * 360.0f * StartRotationRate->GetValue(Owner->EmitterTime, Owner);
			Particle.RotationRate += StartRotRate;
			Particle.BaseRotationRate += StartRotRate;
		}
	}

// --------------------------------------------------------------------------------------------

#if WITH_EDITOR
	bool ParticleModuleRotation::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= StartRotation->Draw(StartRotation, "Start Rotation");

		return bDirty;
	}

	bool ParticleModuleRotationRate::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= StartRotationRate->Draw(StartRotationRate, "Start Rotation Rate");

		return bDirty;
	}
#endif

}  // namespace Drn