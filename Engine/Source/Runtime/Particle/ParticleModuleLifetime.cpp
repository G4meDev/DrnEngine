#include "DrnPCH.h"
#include "ParticleModuleLifetime.h"

namespace Drn
{
	ParticleModuleLifetime::ParticleModuleLifetime()
		: ParticleModule()
		, Lifetime(new ParticleDistributionFloatConstant(1.0f))
	{
		bSpawnModule = true;
	}

	void ParticleModuleLifetime::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			float MaxLifetime = Lifetime->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));
			if(Particle.OneOverMaxLifetime > 0.f)
			{
				// Another module already modified lifetime.
				Particle.OneOverMaxLifetime = 1.f / (MaxLifetime + 1.f / Particle.OneOverMaxLifetime);
			}
			else
			{
				// First module to modify lifetime.
				Particle.OneOverMaxLifetime = MaxLifetime > 0.f ? 1.f / MaxLifetime : 0.f;
			}
			//If the relative time is already > 1.0f then we don't want to be setting it. Some modules use this to mark a particle as dead during spawn.
			Particle.RelativeTime = Particle.RelativeTime > 1.0f ? Particle.RelativeTime : SpawnTime * Particle.OneOverMaxLifetime;
		}
	}

	void ParticleModuleLifetime::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Lifetime = ParticleDistributionFloat::Create(Ar);
		}

		else
		{
			Lifetime->Serialize(Ar);
		}
	}

#if WITH_EDITOR
	bool ParticleModuleLifetime::Draw( ParticleEmitter* Owner )
	{
		drn_check(Lifetime);

		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= Lifetime->Draw(Lifetime, "Lifetime");

		return bDirty;
	}
#endif

}  // namespace Drn