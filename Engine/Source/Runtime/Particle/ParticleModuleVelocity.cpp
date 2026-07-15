#include "DrnPCH.h"
#include "ParticleModuleVelocity.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	void ParticleModuleVelocityBase::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> bWorldSpace;
			Ar >> bApplyOwnerScale;
		}
		else
		{
			Ar << bWorldSpace;
			Ar << bApplyOwnerScale;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleVelocityBase::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = false;

		bDirty |= ImGui::Checkbox("World Space", &bWorldSpace);
		bDirty |= ImGui::Checkbox("Apply Owner Scale", &bApplyOwnerScale);

		return bDirty;
	}
#endif

	ParticleModuleVelocity::ParticleModuleVelocity()
		: ParticleModuleVelocityBase()
		, StartVelocity(new ParticleDistributionVectorConstant(Vector::ZeroVector))
		, StartVelocityRadial(new ParticleDistributionFloatConstant(0.0f))
	{
		bSpawnModule = true;
	}

	void ParticleModuleVelocity::Spawn( ParticleEmitterInstance* Owner, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			Vector Vel = StartVelocity->GetValue(Owner, &GetRandomStream(Owner));
			Vector FromOrigin = (Particle.Location - Owner->EmitterToSimulation.Location()).GetSafeNormal();

			Vector OwnerScale(1.0f);
			if ((bApplyOwnerScale == true) && Owner->Component)
			{
				OwnerScale = Owner->Component->GetWorldTransform().GetScale();
			}

			if (Owner->Emitter->bUseLocalSpace)
			{
				if (bWorldSpace == true)
				{
					Vel = Owner->SimulationToWorld.InverseTransformVector(Vel);
				}
				else
				{
					Vel = Owner->EmitterToSimulation.TransformVector(Vel);
				}
			}
			else if (bWorldSpace == false)
			{
				Vel = Owner->EmitterToSimulation.TransformVector(Vel);
			}
			Vel *= OwnerScale;
			Vel += FromOrigin * StartVelocityRadial->GetValue(Owner, &GetRandomStream(Owner)) * OwnerScale;
			Particle.Velocity		+= Vel;
			Particle.BaseVelocity	+= Vel;
		}
	}

	void ParticleModuleVelocity::Serialize( Archive& Ar )
	{
		ParticleModuleVelocityBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			StartVelocity = ParticleDistributionVector::Create(Ar);
			StartVelocityRadial = ParticleDistributionFloat::Create(Ar);
		}

		else
		{
			StartVelocity->Serialize(Ar);
			StartVelocityRadial->Serialize(Ar);
		}
	}

#if WITH_EDITOR
	bool ParticleModuleVelocity::Draw( ParticleEmitter* Owner )
	{
		drn_check(StartVelocity);
		drn_check(StartVelocityRadial);

		bool bDirty = ParticleModuleVelocityBase::Draw(Owner);
		StartVelocity->Draw(StartVelocity, "Start Velocity");
		StartVelocityRadial->Draw(StartVelocityRadial, "Start Velocity Radial");

		return bDirty;
	}
#endif

}  // namespace Drn