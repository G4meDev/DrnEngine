#include "DrnPCH.h"
#include "ParticleModuleAcceleration.h"

namespace Drn
{
	void ParticleModuleAccelerationBase::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> bWorldSpace;
		}
		else
		{
			Ar << bWorldSpace;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleAccelerationBase::Draw( ParticleEmitter* Owner )
	{
		return ImGui::Checkbox("World Space", &bWorldSpace);
	}
#endif

// -----------------------------------------------------------------------------------------

	ParticleModuleAccelerationConstant::ParticleModuleAccelerationConstant()
		: ParticleModuleAccelerationBase()
		, Acceleration(Vector::ZeroVector)
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleAccelerationConstant::Spawn( ParticleEmitterInstance* Owner, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		if (bWorldSpace && Owner->Emitter->bUseLocalSpace)
		{
			Vector LocalAcceleration = Owner->Component->GetWorldTransform().InverseTransformVector(Acceleration);
			Particle.Velocity		+= LocalAcceleration * SpawnTime;
			Particle.BaseVelocity	+= LocalAcceleration * SpawnTime;
		}
		else
		{
			Vector LocalAcceleration = Acceleration;
			if (Owner->Emitter->bUseLocalSpace)
			{
				LocalAcceleration = Owner->EmitterToSimulation.TransformVector(LocalAcceleration);
			}
			Particle.Velocity		+= LocalAcceleration * SpawnTime;
			Particle.BaseVelocity	+= LocalAcceleration * SpawnTime;
		}
	}

	void ParticleModuleAccelerationConstant::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		if ((!Owner) || (Owner->ActiveParticles <= 0) || (!Owner->ParticleData) || (!Owner->ParticleIndices))
		{
			return;
		}

		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride));
		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride) + PLATFORM_CACHE_LINE_SIZE);

		if (bWorldSpace && Owner->Emitter->bUseLocalSpace)
		{
			Transform Mat = Owner->Component->GetWorldTransform();
			Vector LocalAcceleration = Mat.InverseTransformVector(Acceleration);
			BEGIN_UPDATE_LOOP;
			{
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride));
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);
				Particle.Velocity		+= LocalAcceleration * DeltaTime;
				Particle.BaseVelocity	+= LocalAcceleration * DeltaTime;
			}
			END_UPDATE_LOOP;
		}
		else
		{
			Vector LocalAcceleration = Acceleration;
			if (Owner->Emitter->bUseLocalSpace)
			{
				LocalAcceleration = Owner->EmitterToSimulation.TransformVector(LocalAcceleration);
			}
			BEGIN_UPDATE_LOOP;
			{
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride));
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);
				Particle.Velocity		+= LocalAcceleration * DeltaTime;
				Particle.BaseVelocity	+= LocalAcceleration * DeltaTime;
			}
			END_UPDATE_LOOP;
		}
	}

	void ParticleModuleAccelerationConstant::Serialize( Archive& Ar )
	{
		ParticleModuleAccelerationBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Acceleration;
		}
		else
		{
			Ar << Acceleration;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleAccelerationConstant::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleAccelerationBase::Draw(Owner);
		bDirty |= Acceleration.Draw("Acceleration", "Acceleration", EParameterPopupContext::None);

		return bDirty;
	}
#endif

// -----------------------------------------------------------------------------------------


}  // namespace Drn