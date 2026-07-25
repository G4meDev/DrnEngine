#include "DrnPCH.h"
#include "ParticleModuleSubuv.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleSubuv::ParticleModuleSubuv()
		: ParticleModule()
		, SubImageIndex(new ParticleDistributionFloatConstant(0.0f))
		, bUseUpdate(true)
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleSubuv::CompileModule( ParticleEmitter* Emitter )
	{
		Emitter->bHasSubuv = true;
	}

	void ParticleModuleSubuv::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		const int32 SubuvOffset = Owner->Emitter->GetSubuvOffset();
		if (SubuvOffset)
		{
			SPAWN_INIT;
			{
				SubuvPayloadData* PayloadData = (SubuvPayloadData*)((uint8*)&Particle + SubuvOffset);
				PayloadData->ImageIndex = SubImageIndex->GetValue(Particle.RelativeTime, Owner);
			}
		}
	}

	void ParticleModuleSubuv::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		if (bUseUpdate)
		{
			const int32 SubuvOffset = Owner->Emitter->GetSubuvOffset();
			if (SubuvOffset)
			{
				BEGIN_UPDATE_LOOP;
				{
					SubuvPayloadData* PayloadData = (SubuvPayloadData*)((uint8*)&Particle + SubuvOffset);
					PayloadData->ImageIndex = SubImageIndex->GetValue(Particle.RelativeTime, Owner);
				}
				END_UPDATE_LOOP;
			}
		}
	}

	void ParticleModuleSubuv::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			SubImageIndex = ParticleDistributionFloat::Create(Ar);
			Ar >> bUseUpdate;
		}
		else
		{
			SubImageIndex->Serialize(Ar);
			Ar << bUseUpdate;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleSubuv::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= ImGui::Checkbox("Use Update", &bUseUpdate);
		bDirty |= SubImageIndex->Draw(SubImageIndex, "Sub Image Index");

		return bDirty;
	}
#endif

}  // namespace Drn