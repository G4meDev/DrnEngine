#include "DrnPCH.h"
#include "ParticleModuleAcceleration.h"

namespace Drn
{
	void ParticleModuleAccelerationBase::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bWorldSpace;
		}
		else
		{
			Ar << bWorldSpace;
		}
	}

// -----------------------------------------------------------------------------------------

	ParticleModuleAccelerationConstant::ParticleModuleAccelerationConstant()
		: ParticleModuleAccelerationBase()
		, Acceleration(Vector::ZeroVector)
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleAccelerationConstant::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
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

// -----------------------------------------------------------------------------------------

	ParticleModuleAcceleration::ParticleModuleAcceleration()
		: ParticleModuleAccelerationBase()
		, bApplyOwnerScale(false)
		, Acceleration(new ParticleDistributionVectorConstant(Vector::ZeroVector))
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleAcceleration::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		PARTICLE_ELEMENT(Vector, UsedAcceleration);
		UsedAcceleration = Acceleration->GetValue(Owner->EmitterTime, Owner);
		if ((bApplyOwnerScale == true) && Owner && Owner->Component)
		{
			Vector Scale = Owner->Component->GetWorldTransform().GetScale();
			UsedAcceleration *= Scale;
		}

		if (bWorldSpace && Owner->Emitter->bUseLocalSpace)
		{
			Vector TempUsedAcceleration = Owner->Component->GetWorldTransform().InverseTransformVector(UsedAcceleration);
			Particle.Velocity		+= TempUsedAcceleration * SpawnTime;
			Particle.BaseVelocity	+= TempUsedAcceleration * SpawnTime;
		}
		else
		{
			if (Owner->Emitter->bUseLocalSpace)
			{
				UsedAcceleration = Owner->EmitterToSimulation.TransformVector(UsedAcceleration);
			}
			Particle.Velocity		+= UsedAcceleration * SpawnTime;
			Particle.BaseVelocity	+= UsedAcceleration * SpawnTime;
		}
	}

	void ParticleModuleAcceleration::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		if (!Owner || !Owner->HasActiveParticles() || !Owner->ParticleData || !Owner->ParticleIndices)
		{
			return;
		}

		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride));
		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride) + PLATFORM_CACHE_LINE_SIZE);
		if (bWorldSpace && Owner->Emitter->bUseLocalSpace)
		{
			Transform Mat = Owner->Component->GetWorldTransform();
			BEGIN_UPDATE_LOOP;
			{
				Vector& UsedAcceleration = *((Vector*)(ParticleBase + CurrentOffset));																\
				Vector TransformedUsedAcceleration = Mat.InverseTransformVector(UsedAcceleration);
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride));
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);
				Particle.Velocity		+= TransformedUsedAcceleration * DeltaTime;
				Particle.BaseVelocity	+= TransformedUsedAcceleration * DeltaTime;
			}
			END_UPDATE_LOOP;
		}
		else
		{
			BEGIN_UPDATE_LOOP;
			{
				Vector& UsedAcceleration = *((Vector*)(ParticleBase + CurrentOffset));																\
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride));
				ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);
				Particle.Velocity		+= UsedAcceleration * DeltaTime;
				Particle.BaseVelocity	+= UsedAcceleration * DeltaTime;
			}
			END_UPDATE_LOOP;
		}
	}

	void ParticleModuleAcceleration::Serialize( Archive& Ar )
	{
		ParticleModuleAccelerationBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bApplyOwnerScale;
			Acceleration = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			Ar << bApplyOwnerScale;
			Acceleration->Serialize(Ar);
		}
	}

// -----------------------------------------------------------------------------------------

	ParticleModuleAccelerationOverLife::ParticleModuleAccelerationOverLife()
		: ParticleModuleAccelerationBase()
		, AccelerationOverLife(new ParticleDistributionVectorConstant(Vector::ZeroVector))
	{
		bUpdateModule = true;
	}

	void ParticleModuleAccelerationOverLife::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		if (bWorldSpace && Owner->Emitter->bUseLocalSpace)
		{
			Transform Mat = Owner->Component->GetWorldTransform();
			BEGIN_UPDATE_LOOP;
				Vector Accel = AccelerationOverLife->GetValue(Particle.RelativeTime, Owner);
				Accel = Mat.InverseTransformVector(Accel);
				Particle.Velocity		+= Accel * DeltaTime;
				Particle.BaseVelocity	+= Accel * DeltaTime;
			END_UPDATE_LOOP;
		}
		else
		{
			BEGIN_UPDATE_LOOP;
			Vector Accel = AccelerationOverLife->GetValue(Particle.RelativeTime, Owner);
			Particle.Velocity		+= Accel * DeltaTime;
			Particle.BaseVelocity	+= Accel * DeltaTime;
			END_UPDATE_LOOP;
		}
	}

	void ParticleModuleAccelerationOverLife::Serialize( Archive& Ar )
	{
		ParticleModuleAccelerationBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			AccelerationOverLife = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			AccelerationOverLife->Serialize(Ar);
		}
	}

// -----------------------------------------------------------------------------------------

	ParticleModuleDrag::ParticleModuleDrag()
		: ParticleModuleAccelerationBase()
		, DragCoefficient(new ParticleDistributionFloatConstant(1.0f))
	{
		bUpdateModule = true;
	}

	void ParticleModuleDrag::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		BEGIN_UPDATE_LOOP;
		{
			Vector Drag  = Particle.Velocity * -DragCoefficient->GetValue(Particle.RelativeTime, Owner);
			Particle.Velocity		+= Drag * DeltaTime;
			Particle.BaseVelocity	+= Drag * DeltaTime;
		}
		END_UPDATE_LOOP;
	}

	void ParticleModuleDrag::Serialize( Archive& Ar )
	{
		ParticleModuleAccelerationBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			DragCoefficient = ParticleDistributionFloat::Create(Ar);
		}
		else
		{
			DragCoefficient->Serialize(Ar);
		}
	}

// -----------------------------------------------------------------------------------------

#if WITH_EDITOR
	bool ParticleModuleAccelerationBase::Draw( ParticleEmitter* Owner )
	{
		return ImGui::Checkbox("World Space", &bWorldSpace);
	}

	bool ParticleModuleAccelerationConstant::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleAccelerationBase::Draw(Owner);
		bDirty |= Acceleration.Draw("Acceleration", "Acceleration", EParameterPopupContext::None);

		return bDirty;
	}

	bool ParticleModuleAcceleration::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleAccelerationBase::Draw(Owner);
		bDirty |= ImGui::Checkbox("Apply Owner Scale", &bApplyOwnerScale);
		bDirty |= Acceleration->Draw(Acceleration, "Acceleration");

		return bDirty;
	}

	bool ParticleModuleAccelerationOverLife::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleAccelerationBase::Draw(Owner);
		bDirty |= AccelerationOverLife->Draw(AccelerationOverLife, "Acceleration Over Life");

		return bDirty;
	}

	bool ParticleModuleDrag::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleAccelerationBase::Draw(Owner);
		bDirty |= DragCoefficient->Draw(DragCoefficient, "Drag Coefficient");

		return bDirty;
	}
#endif

}  // namespace Drn