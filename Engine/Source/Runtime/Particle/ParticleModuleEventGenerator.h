#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct ParticleEvent_GenerateInfo
	{
		EParticleEventType Type;

		int32 Frequency;
		int32 ParticleFrequency;
		bool FirstTimeOnly;
		bool LastTimeOnly;
		bool UseReflectedImpactVector;
		bool bUseOrbitOffset;
		std::string CustomName;

		//std::vector<class UParticleModuleEventSendToGame*> ParticleModuleEventsToSendToGame;

		ParticleEvent_GenerateInfo()
			: Type(EParticleEventType::EPET_Any)
			, Frequency(0)
			, ParticleFrequency(0)
			, FirstTimeOnly(false)
			, LastTimeOnly(false)
			, UseReflectedImpactVector(false)
			, bUseOrbitOffset(false)
		{
		}

		friend Archive& operator>>( Archive& Ar, ParticleEvent_GenerateInfo& Event )
		{
			Ar >> Event.Frequency;
			Ar >> Event.ParticleFrequency;
			Ar >> Event.FirstTimeOnly;
			Ar >> Event.LastTimeOnly;
			Ar >> Event.UseReflectedImpactVector;
			Ar >> Event.bUseOrbitOffset;
			Ar >> Event.CustomName;

			return Ar;
		}

		friend Archive& operator<<( Archive& Ar, ParticleEvent_GenerateInfo& Event )
		{
			Ar << Event.Frequency;
			Ar << Event.ParticleFrequency;
			Ar << Event.FirstTimeOnly;
			Ar << Event.LastTimeOnly;
			Ar << Event.UseReflectedImpactVector;
			Ar << Event.bUseOrbitOffset;
			Ar << Event.CustomName;

			return Ar;
		}

#if WITH_EDITOR
		bool Draw();
#endif
	};

	class ParticleModuleEventGenerator : public ParticleModule
	{
	public:
		ParticleModuleEventGenerator();

		std::vector<ParticleEvent_GenerateInfo> Events;

		virtual void Serialize( Archive& Ar ) override;
		virtual EParticleModule	GetModuleType() const override { return EParticleModule::EventGenerator; }

		virtual uint32 RequiredBytes() override;
		virtual uint32 RequiredBytesPerInstance() override;
		virtual uint32 PrepPerInstanceBlock(ParticleEmitterInstance* Owner, void* InstData) override;

		virtual bool HandleParticleSpawned(ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, BaseParticle* NewParticle);

		virtual bool HandleParticleKilled(ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, BaseParticle* DeadParticle);

		virtual bool HandleParticleCollision(ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, 
			ParticleCollisionPayload* CollidePayload, HitResult* Hit, BaseParticle* CollideParticle, Vector& CollideDirection);

		virtual bool HandleParticleBurst(ParticleEmitterInstance* Owner, ParticleEventInstancePayload* EventPayload, const int32 ParticleCount);

#if WITH_EDITOR
		virtual bool Draw(ParticleEmitter* Owner) override;
#endif
	};
}