#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleModuleEventReceiverBase : public ParticleModule
	{
	public:
		ParticleModuleEventReceiverBase();

		EParticleEventType EventGeneratorType;
		std::string EventName;

		virtual void Serialize( Archive& Ar ) override;

		virtual bool WillProcessParticleEvent(EParticleEventType InEventType)
		{
			if ((EventGeneratorType == EPET_Any) || (InEventType == EventGeneratorType))
			{
				return true;
			}

			return false;
		}

		virtual bool ProcessParticleEvent(ParticleEmitterInstance* Owner, ParticleEventData& InEvent, float DeltaTime)
		{
			return false;
		}

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

	class ParticleModuleEventReceiverKillParticles : public ParticleModuleEventReceiverBase
	{
	public:
		ParticleModuleEventReceiverKillParticles();

		bool bStopSpawning;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::EventReceiverKillParticles; }

		virtual bool ProcessParticleEvent(ParticleEmitterInstance* Owner, ParticleEventData& InEvent, float DeltaTime) override;

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};

}