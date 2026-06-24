#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"

namespace Drn
{
	class ParticleSystemComponent;

	class Particle : public Actor
	{
	public:
		Particle();
		virtual ~Particle();

		virtual void Serialize(Archive& Ar) override;
		virtual void Tick(float DeltaTime) override;

		inline ParticleSystemComponent* GetParticleSystemComponenet() { return m_ParticleSystemComponenet.get(); }

		inline virtual EActorType GetActorType() override { return EActorType::Particle; }

	protected:

		std::unique_ptr<ParticleSystemComponent> m_ParticleSystemComponenet;

	private:


	};
}