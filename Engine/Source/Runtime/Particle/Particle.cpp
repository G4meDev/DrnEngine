#include "DrnPCH.h"
#include "Particle.h"
#include "Runtime/Particle/ParticleSystemComponent.h"

namespace Drn
{
	Particle::Particle()
	{
		m_ParticleSystemComponenet = std::make_unique<ParticleSystemComponent>();
		SetRootComponent( m_ParticleSystemComponenet.get() );

#if WITH_EDITOR
		m_ParticleSystemComponenet->SetComponentLabel( "Particle" );
#endif
	}

	Particle::~Particle()
	{
		
	}

	void Particle::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);
		m_ParticleSystemComponenet->Serialize(Ar);
	}

	void Particle::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);


	}

}