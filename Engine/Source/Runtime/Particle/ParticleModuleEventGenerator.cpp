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
		drn_check(Owner && EventPayload && NewParticle);

		EventPayload->SpawnTrackingCount++;

		bool bProcessed = false;
		for (int32 EventIndex = 0; EventIndex < Events.size(); EventIndex++)
		{
			ParticleEvent_GenerateInfo& EventGenInfo = Events[EventIndex];
			if (EventGenInfo.Type == EPET_Spawn)
			{
				if (EventGenInfo.Frequency == 0 || (EventPayload->SpawnTrackingCount % EventGenInfo.Frequency) == 0)
				{
					//Vector ParticleLocation = EventGenInfo.bUseOrbitOffset ? Owner->GetParticleLocationWithOrbitOffset(NewParticle) : NewParticle->Location;
					Vector ParticleLocation = NewParticle->Location;

					Owner->Component->ReportEventSpawn(EventGenInfo.CustomName, Owner->EmitterTime, 
						ParticleLocation, NewParticle->Velocity);
					bProcessed = true;
				}
			}
		}

		return bProcessed;
	}

	bool ParticleModuleEventGenerator::HandleParticleKilled( ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, BaseParticle* DeadParticle )
	{
		drn_check(Owner && EventPayload && DeadParticle);

		EventPayload->DeathTrackingCount++;

		bool bProcessed = false;
		for (int32 EventIndex = 0; EventIndex < Events.size(); EventIndex++)
		{
			ParticleEvent_GenerateInfo& EventGenInfo = Events[EventIndex];
			if (EventGenInfo.Type == EPET_Death)
			{
				if (EventGenInfo.Frequency == 0 || (EventPayload->DeathTrackingCount % EventGenInfo.Frequency) == 0)
				{
					//FVector ParticleLocation = EventGenInfo.bUseOrbitOffset ? Owner->GetParticleLocationWithOrbitOffset(DeadParticle) : DeadParticle->Location;
					Vector ParticleLocation = DeadParticle->Location;

					Owner->Component->ReportEventDeath(EventGenInfo.CustomName, 
						Owner->EmitterTime, ParticleLocation, DeadParticle->Velocity, 
						DeadParticle->RelativeTime);
					bProcessed = true;
				}
			}
		}

		return bProcessed;
	}

	bool ParticleModuleEventGenerator::HandleParticleCollision( ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload,
		ParticleCollisionPayload* CollidePayload, HitResult* Hit, BaseParticle* CollideParticle, Vector& CollideDirection )
	{
		drn_check(Owner && EventPayload && CollideParticle);

		EventPayload->CollisionTrackingCount++;

		bool bProcessed = false;
		for (int32 EventIndex = 0; EventIndex < Events.size(); EventIndex++)
		{
			ParticleEvent_GenerateInfo& EventGenInfo = Events[EventIndex];
			if (EventGenInfo.Type == EPET_Collision)
			{
				if (EventGenInfo.FirstTimeOnly == true)
				{
					if ((CollideParticle->Flags & STATE_Particle_CollisionHasOccurred) != 0)
					{
						continue;
					}
				}
				else
				if (EventGenInfo.LastTimeOnly == true)
				{
					if (CollidePayload->UsedCollisions != 0)
					{
						continue;
					}
				}

				if (EventGenInfo.Frequency == 0 || (EventPayload->CollisionTrackingCount % EventGenInfo.Frequency) == 0)
				{
					Owner->Component->ReportEventCollision(
						EventGenInfo.CustomName, 
						Owner->EmitterTime, 
						Hit->Location,
						CollideDirection, 
						CollideParticle->Velocity, 
						CollideParticle->RelativeTime, 
						Hit->Normal,
						1, 0, "",
//						Hit->Time, 
//						Hit->Item, 
//						Hit->BoneName,
						Hit->PhysMaterial);
					bProcessed = true;
				}
			}
		}

		return bProcessed;
	}

	bool ParticleModuleEventGenerator::HandleParticleBurst( ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, const int32 ParticleCount )
	{
		drn_check(Owner && EventPayload);

		++EventPayload->BurstTrackingCount;

		bool bProcessed = false;
		for (int32 EventIndex = 0; EventIndex < Events.size(); ++EventIndex)
		{
			ParticleEvent_GenerateInfo& EventGenInfo = Events[EventIndex];
			if (EventGenInfo.Type == EPET_Burst)
			{
				if (EventGenInfo.Frequency == 0 || (EventPayload->BurstTrackingCount % EventGenInfo.Frequency) == 0)
				{
					Owner->Component->ReportEventBurst(EventGenInfo.CustomName, Owner->EmitterTime, ParticleCount, 
						Owner->Location);
					bProcessed = true;
				}
			}
		}

		return bProcessed;
	}


#if WITH_EDITOR
	bool ParticleEvent_GenerateInfo::Draw()
	{
		bool bDirty = false;

		const char* const Options[] = { "Any", "Spawn", "Death", "Collision", "Burst", "Blueprint" };
		int32 Selected = Type;
		bDirty |= ImGui::Combo("Generator Type", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			Type = (EParticleEventType)Selected;
		}

		const int32 TextCharLimit = 64;
		char InputText[TextCharLimit];
		strcpy_s(InputText, sizeof(InputText), CustomName.c_str());

		if ( ImGui::InputText( "Event Name", InputText, TextCharLimit ) )
		{
			CustomName = InputText;
			bDirty = true;
		}

		bDirty |= ImGui::InputInt("Frequency", &Frequency);
		bDirty |= ImGui::InputInt("Particle Frequency", &ParticleFrequency);
		bDirty |= ImGui::Checkbox("First Time Only", &FirstTimeOnly);
		bDirty |= ImGui::Checkbox("Last Time Only", &LastTimeOnly);
		bDirty |= ImGui::Checkbox("Use Reflected Impact Vector", &UseReflectedImpactVector);
		bDirty |= ImGui::Checkbox("Use Orbit Offset", &bUseOrbitOffset);

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