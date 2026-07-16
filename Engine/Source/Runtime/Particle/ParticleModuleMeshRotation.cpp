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
		Emitter->bHasMeshRotation = true;
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

#if WITH_EDITOR
	bool ParticleModuleMeshRotation::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= ImGui::Checkbox("Inherit Parent", &bInheritParent);
		bDirty |= StartRotation->Draw(StartRotation, "Start Rotation");

		return bDirty;
	}
#endif

// ----------------------------------------------------------------------------------------------------------

	ParticleModuleMeshRotationRate::ParticleModuleMeshRotationRate()
		: ParticleModule()
		, StartRotationRate(new ParticleDistributionVectorConstant(Vector::ZeroVector))
	{
		bSpawnModule = true;
	}

	void ParticleModuleMeshRotationRate::CompileModule( ParticleEmitter* Emitter )
	{
		Emitter->bHasMeshRotation = true;
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

#if WITH_EDITOR
	bool ParticleModuleMeshRotationRate::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= StartRotationRate->Draw(StartRotationRate, "Start Rotation Rate");

		return bDirty;
	}
#endif

}  // namespace Drn