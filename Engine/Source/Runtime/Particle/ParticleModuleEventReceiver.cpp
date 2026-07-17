#include "DrnPCH.h"
#include "ParticleModuleEventReceiver.h"

namespace Drn
{
	ParticleModuleEventReceiverBase::ParticleModuleEventReceiverBase()
		: ParticleModule()
		, EventGeneratorType(EParticleEventType::EPET_Any)
		, EventName("None")
	{}

	void ParticleModuleEventReceiverBase::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> *(uint32*)&EventGeneratorType;
			Ar >> EventName;
		}
		else
		{
			Ar << (uint32)EventGeneratorType;
			Ar << EventName;
		}
	}

// -----------------------------------------------------------------------------------------------------

	ParticleModuleEventReceiverKillParticles::ParticleModuleEventReceiverKillParticles()
		: ParticleModuleEventReceiverBase()
		, bStopSpawning(false)
	{
		bEventReciverModule = true;
	}

	void ParticleModuleEventReceiverKillParticles::Serialize( Archive& Ar )
	{
		ParticleModuleEventReceiverBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bStopSpawning;
		}
		else
		{
			Ar << bStopSpawning;
		}
	}

	bool ParticleModuleEventReceiverKillParticles::ProcessParticleEvent( ParticleEmitterInstance* Owner, ParticleEventData& InEvent, float DeltaTime )
	{
		if ((InEvent.EventName == EventName) && ((EventGeneratorType == EPET_Any) || (EventGeneratorType == InEvent.Type)))
		{
			Owner->KillParticlesForced(true);
			if (bStopSpawning == true)
			{
				Owner->bHaltSpawning = true;
			}
			return true;
		}

		return false;
	}

#if WITH_EDITOR
	bool ParticleModuleEventReceiverBase::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		const char* const Options[] = { "Any", "Spawn", "Death", "Collision", "Burst", "Blueprint" };
		int32 Selected = EventGeneratorType;
		bDirty |= ImGui::Combo("Generator Type", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			EventGeneratorType = (EParticleEventType)Selected;
		}

		const int32 TextCharLimit = 64;
		char InputText[TextCharLimit];
		strcpy_s(InputText, sizeof(InputText), EventName.c_str());

		if ( ImGui::InputText( "Event Name", InputText, TextCharLimit ) )
		{
			EventName = InputText;
			bDirty = true;
		}

		return bDirty;
	}

	bool ParticleModuleEventReceiverKillParticles::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleEventReceiverBase::Draw(Owner);

		ImGui::Checkbox("Stop Spawning", &bStopSpawning);
		
		return bDirty;
	}
#endif

}  // namespace Drn