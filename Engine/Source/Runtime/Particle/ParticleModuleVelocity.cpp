#include "DrnPCH.h"
#include "ParticleModuleVelocity.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	void ParticleModuleVelocityBase::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

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

	void ParticleModuleVelocity::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			Vector Vel = StartVelocity->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));
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
			Vel += FromOrigin * StartVelocityRadial->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner)) * OwnerScale;
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

// -----------------------------------------------------------------------------------------------------------------------------------

	ParticleModuleVelocityOverLifetime::ParticleModuleVelocityOverLifetime()
		: ParticleModuleVelocityBase()
		, bAbsolute(false)
		, VelocityOverLifetime(new ParticleDistributionVectorConstant(Vector::ZeroVector))
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleVelocityOverLifetime::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		if (bAbsolute)
		{
			SPAWN_INIT;
			Vector OwnerScale(1.0f);
			if ((bApplyOwnerScale == true) && Owner && Owner->Component)
			{
				OwnerScale = Owner->Component->GetWorldTransform().GetScale();
			}
			Vector Vel = VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner)) * OwnerScale;
			Particle.Velocity		= Vel;
			Particle.BaseVelocity	= Vel;
		}
	}

	void ParticleModuleVelocityOverLifetime::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		Vector OwnerScale(1.0f);
		const Transform& OwnerTM = Owner->Component->GetWorldTransform();
		if (bApplyOwnerScale == true)
		{
			OwnerScale = OwnerTM.GetScale();
		}
		if (bAbsolute)
		{
			if (!Owner->Emitter->bUseLocalSpace)
			{
				if (bWorldSpace == false)
				{
					Vector Vel;
					const Matrix LocalToWorld = OwnerTM.ToMatrixNoScale();
					BEGIN_UPDATE_LOOP;
					{
						Vel = VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner));
						Particle.Velocity = LocalToWorld.TransformVector(Vel) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
				else
				{
					BEGIN_UPDATE_LOOP;
					{
						Particle.Velocity = VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner)) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
			}
			else
			{
				if (bWorldSpace == false)
				{
					BEGIN_UPDATE_LOOP;
					{
						Particle.Velocity = VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner)) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
				else
				{
					Vector Vel;
					const Matrix LocalToWorld = OwnerTM.ToMatrixNoScale();
					const Matrix InvMat = LocalToWorld.Inverse();
					BEGIN_UPDATE_LOOP;
					{
						Vel = VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner));
						Particle.Velocity = InvMat.TransformVector(Vel) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
			}
		}
		else
		{
			if (Owner->Emitter->bUseLocalSpace == false)
			{
				Vector Vel;
				if (bWorldSpace == false)
				{
					const Matrix LocalToWorld = OwnerTM.ToMatrixNoScale();
					BEGIN_UPDATE_LOOP;
					{
						Vel = VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner));
						Particle.Velocity *= LocalToWorld.TransformVector(Vel) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
				else
				{
					BEGIN_UPDATE_LOOP;
					{
						Particle.Velocity *= VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner)) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
			}
			else
			{
				if (bWorldSpace == false)
				{
					BEGIN_UPDATE_LOOP;
					{
						Particle.Velocity *= VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner)) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
				else
				{
					Vector Vel;
					const Matrix LocalToWorld = OwnerTM.ToMatrixNoScale();
					const Matrix InvMat = LocalToWorld.Inverse();
					BEGIN_UPDATE_LOOP;
					{
						Vel = VelocityOverLifetime->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner));
						Particle.Velocity *= InvMat.TransformVector(Vel) * OwnerScale;
					}
					END_UPDATE_LOOP;
				}
			}
		}
	}

	void ParticleModuleVelocityOverLifetime::Serialize( Archive& Ar )
	{
		ParticleModuleVelocityBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			VelocityOverLifetime = ParticleDistributionVector::Create(Ar);
			Ar >> bAbsolute;
		}

		else
		{
			VelocityOverLifetime->Serialize(Ar);
			Ar << bAbsolute;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleVelocityOverLifetime::Draw( ParticleEmitter* Owner )
	{
		drn_check(VelocityOverLifetime);

		bool bDirty = ParticleModuleVelocityBase::Draw(Owner);
		
		ImGui::Checkbox("Absolute", &bAbsolute);
		VelocityOverLifetime->Draw(VelocityOverLifetime, "Velocity Over Lifetime");

		return bDirty;
	}
#endif

}  // namespace Drn