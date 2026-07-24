#include "DrnPCH.h"
#include "ParticleModuleMeshRotation.h"

namespace Drn
{
	ParticleModuleMeshRotation::ParticleModuleMeshRotation()
		: ParticleModule()
		, bInheritParent(false)
		, StartRotation(new ParticleDistributionVectorConstant(Vector::ZeroVector))
	{
		bSpawnModule = true;
	}

	void ParticleModuleMeshRotation::CompileModule( ParticleEmitter* Emitter )
	{
		if (Emitter->IsMeshEmitter())
		{
			Emitter->bHasMeshRotation = true;
		}

		else
		{
			bValid = false;
		}
	}

	void ParticleModuleMeshRotation::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			const int32 MeshRotationOffset = Owner->Emitter->GetMeshRotationOffset();
			if (MeshRotationOffset)
			{
				Vector Rotation = StartRotation->GetValue(Owner->EmitterTime, Owner);
				//if (bInheritParent)
				//{
				//	Quat Rotator = Owner->Component->GetWorldRotation();
				//	Vector		ParentAffectedRotation	= Rotator.Euler();
				//	Rotation.X	+= ParentAffectedRotation.X / 360.0f;
				//	Rotation.Y	+= ParentAffectedRotation.Y / 360.0f;
				//	Rotation.Z	+= ParentAffectedRotation.Z / 360.0f;
				//}

				MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
				PayloadData->InitRotation = Rotation * 360.0f;
				PayloadData->Rotation += PayloadData->InitRotation;
			}
		}
	}

	void ParticleModuleMeshRotation::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bInheritParent;
			StartRotation = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			Ar << bInheritParent;
			StartRotation->Serialize(Ar);
		}
	}

// ----------------------------------------------------------------------------------------------------------

	ParticleModuleMeshRotationRate::ParticleModuleMeshRotationRate()
		: ParticleModule()
		, StartRotationRate(new ParticleDistributionVectorConstant(Vector::ZeroVector))
	{
		bSpawnModule = true;
	}

	void ParticleModuleMeshRotationRate::CompileModule( ParticleEmitter* Emitter )
	{
		if (Emitter->IsMeshEmitter())
		{
			Emitter->bHasMeshRotation = true;
		}

		else
		{
			bValid = false;
		}
	}

	void ParticleModuleMeshRotationRate::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			const int32 MeshRotationOffset = Owner->Emitter->GetMeshRotationOffset();
			if (MeshRotationOffset)
			{
				Vector StartRate = StartRotationRate->GetValue(Owner->EmitterTime, Owner);
				Vector StartValue = StartRate * 360.0f;

				MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
				PayloadData->RotationRateBase	+= StartValue;
				PayloadData->RotationRate		+= StartValue;
			}
		}
	}

	void ParticleModuleMeshRotationRate::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			StartRotationRate = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			StartRotationRate->Serialize(Ar);
		}
	}

// ---------------------------------------------------------------------------------------

	ParticleModuleMeshRotationRateOverLifetime::ParticleModuleMeshRotationRateOverLifetime()
		: ParticleModule()
		, RotationRate(new ParticleDistributionVectorConstant(Vector::ZeroVector))
		, bScaleRotationRate(false)
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleMeshRotationRateOverLifetime::CompileModule( ParticleEmitter* Emitter )
	{
		if (Emitter->IsMeshEmitter())
		{
			Emitter->bHasMeshRotation = true;
		}

		else
		{
			bValid = false;
		}
	}

	void ParticleModuleMeshRotationRateOverLifetime::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		const int32 MeshRotationOffset = Owner->Emitter->GetMeshRotationOffset();
		if (MeshRotationOffset)
		{
			SPAWN_INIT;
			{
				MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
				Vector RateValue = RotationRate->GetValue(Particle.RelativeTime, Owner);
				RateValue *= 360.0f;

				if (bScaleRotationRate)
				{
					PayloadData->RotationRate *= RateValue;
				}
				else
				{
					PayloadData->RotationRate += RateValue;
				}
			}
		}
	}

	void ParticleModuleMeshRotationRateOverLifetime::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		const int32 MeshRotationOffset = Owner->Emitter->GetMeshRotationOffset();
		if (MeshRotationOffset)
		{
			MeshRotationPayloadData* PayloadData;
			Vector RateValue;
			if (!bScaleRotationRate)
			{
				BEGIN_UPDATE_LOOP;
				{
					PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
					RateValue = RotationRate->GetValue(Particle.RelativeTime, Owner);
					RateValue.X *= 360.0f;
					PayloadData->RotationRate += RateValue;
				}
				END_UPDATE_LOOP;
			}
			else
			{
				BEGIN_UPDATE_LOOP;
				{
					PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
					RateValue = RotationRate->GetValue(Particle.RelativeTime, Owner);
					RateValue.X *= 360.0f;
					PayloadData->RotationRate *= RateValue;
				}
				END_UPDATE_LOOP;
			}
		}
	}

	void ParticleModuleMeshRotationRateOverLifetime::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			RotationRate = ParticleDistributionVector::Create(Ar);
			Ar >> bScaleRotationRate;
		}
		else
		{
			RotationRate->Serialize(Ar);
			Ar << bScaleRotationRate;
		}
	}

// ---------------------------------------------------------------------------------------

	ParticleModuleMeshRotationRateMultiplyLifetime::ParticleModuleMeshRotationRateMultiplyLifetime()
		: ParticleModule()
		, LifeMultiplier(new ParticleDistributionVectorConstant(Vector::OneVector))
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleMeshRotationRateMultiplyLifetime::CompileModule( ParticleEmitter* Emitter )
	{
		if (Emitter->IsMeshEmitter())
		{
			Emitter->bHasMeshRotation = true;
		}

		else
		{
			bValid = false;
		}
	}

	void ParticleModuleMeshRotationRateMultiplyLifetime::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		const int32 MeshRotationOffset = Owner->Emitter->GetMeshRotationOffset();
		if (MeshRotationOffset)
		{
			SPAWN_INIT;
			{
				MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
				Vector RateScale = LifeMultiplier->GetValue(Particle.RelativeTime, Owner);
				PayloadData->RotationRate *= RateScale;
			}
		}
	}

	void ParticleModuleMeshRotationRateMultiplyLifetime::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		const int32 MeshRotationOffset = Owner->Emitter->GetMeshRotationOffset();
		if (MeshRotationOffset)
		{
			BEGIN_UPDATE_LOOP;
			{
				MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
				Vector RateScale = LifeMultiplier->GetValue(Particle.RelativeTime, Owner);
				PayloadData->RotationRate *= RateScale;
			}
			END_UPDATE_LOOP;
		}
	}

	void ParticleModuleMeshRotationRateMultiplyLifetime::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			LifeMultiplier = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			LifeMultiplier->Serialize(Ar);
		}
	}

// ---------------------------------------------------------------------------------------

#if WITH_EDITOR
	bool ParticleModuleMeshRotationRate::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= StartRotationRate->Draw(StartRotationRate, "Start Rotation Rate");

		return bDirty;
	}

	bool ParticleModuleMeshRotation::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= ImGui::Checkbox("Inherit Parent", &bInheritParent);
		bDirty |= StartRotation->Draw(StartRotation, "Start Rotation");

		return bDirty;
	}

	bool ParticleModuleMeshRotationRateOverLifetime::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= RotationRate->Draw(RotationRate, "Rotation Rate");
		bDirty |= ImGui::Checkbox("Scale Rotation Rate", &bScaleRotationRate);

		return bDirty;
	}

	bool ParticleModuleMeshRotationRateMultiplyLifetime::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= LifeMultiplier->Draw(LifeMultiplier, "Life Multiplier");

		return bDirty;
	}
#endif



}  // namespace Drn