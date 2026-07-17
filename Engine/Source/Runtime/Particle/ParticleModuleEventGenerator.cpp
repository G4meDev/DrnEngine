#include "DrnPCH.h"
#include "ParticleModuleEventGenerator.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleEventGenerator::ParticleModuleEventGenerator() : ParticleModule()
	{
		bEventGenerateModule = true;
	}

	void ParticleModuleEventGenerator::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			int32 EventCount = 0;
			Ar >> EventCount;
			Events.resize(EventCount);
			for (int32 i = 0; i < EventCount; i++)
			{
				Ar >> Events[i];
			}
		}
		else
		{
			const int32 EventCount = Events.size();
			Ar << EventCount;
			for (int32 i = 0; i < EventCount; i++)
			{
				Ar << Events[i];
			}
		}
	}

	uint32 ParticleModuleEventGenerator::RequiredBytes() { return 0; }

	uint32 ParticleModuleEventGenerator::RequiredBytesPerInstance() { return sizeof(ParticleEventInstancePayload); }

	uint32 ParticleModuleEventGenerator::PrepPerInstanceBlock( ParticleEmitterInstance* Owner, void* InstData )
	{
		ParticleEventInstancePayload* Payload = (ParticleEventInstancePayload*)InstData;
		if (Payload)
		{
			for (int32 EventGenIndex = 0; EventGenIndex < Events.size(); EventGenIndex++)
			{
				switch (Events[EventGenIndex].Type)
				{
				case EPET_Spawn:		Payload->bSpawnEventsPresent = true;		break;
				case EPET_Death:		Payload->bDeathEventsPresent = true;		break;
				case EPET_Collision:	Payload->bCollisionEventsPresent = true;	break;
				case EPET_Burst:		Payload->bBurstEventsPresent = true;		break;
				}
			}
			return 0;
		}

		return 0xffffffff;
	}

	bool ParticleModuleEventGenerator::HandleParticleSpawned( ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, BaseParticle* NewParticle )
	{
		
		return true;
	}

	bool ParticleModuleEventGenerator::HandleParticleKilled( ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, BaseParticle* DeadParticle )
	{
		
		return true;
	}

	bool ParticleModuleEventGenerator::HandleParticleCollision( ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload,
		ParticleCollisionPayload* CollidePayload, HitResult* Hit, BaseParticle* CollideParticle, Vector& CollideDirection )
	{
		
		return true;
	}

	bool ParticleModuleEventGenerator::HandleParticleBurst( ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, const int32 ParticleCount )
	{
		
		return true;
	}


#if WITH_EDITOR
	bool ParticleEvent_GenerateInfo::Draw()
	{
		bool bDirty = false;

		bDirty |= ImGui::InputInt("Frequency", &Frequency);
		bDirty |= ImGui::InputInt("Particle Frequency", &ParticleFrequency);
		bDirty |= ImGui::Checkbox("First Time Only", &FirstTimeOnly);
		bDirty |= ImGui::Checkbox("Last Time Only", &LastTimeOnly);
		bDirty |= ImGui::Checkbox("Use Reflected Impact Vector", &UseReflectedImpactVector);
		bDirty |= ImGui::Checkbox("Use Orbit Offset", &bUseOrbitOffset);

		const int32 TextCharLimit = 64;
		char InputText[TextCharLimit];
		strcpy_s(InputText, sizeof(InputText), CustomName.c_str());

		if ( ImGui::InputText( "Event Name", InputText, TextCharLimit ) )
		{
			CustomName = InputText;
			bDirty = true;
		}

		return bDirty;
	}

	bool ParticleModuleEventGenerator::Draw(ParticleEmitter* Owner)
	{
		bool bDirty = ParticleModule::Draw(Owner);

		if (ImGui::Button("Add"))
		{
			Events.push_back({});
			bDirty = true;
		}

		if (ImGui::Button("Clear"))
		{
			Events.clear();
			bDirty = true;
		}

		for (int32 EventIndex = 0; EventIndex < Events.size(); EventIndex++)
		{
			ImGui::PushID(EventIndex);

			bDirty |= Events[EventIndex].Draw();
			ImGui::Separator();

			ImGui::PopID();
		}

		return bDirty;
	}
#endif

}  // namespace Drn