#include "DrnPCH.h"
#include "Controller.h"

namespace Drn
{
	Controller::Controller()
		: Actor()
	{
		m_RootComponent = std::make_unique<class SceneComponent>();
		m_RootComponent->SetComponentLabel( "RootComponent" );
		SetRootComponent(m_RootComponent.get());
	}

	Controller::~Controller()
	{
		
	}

	void Controller::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);


	}

	void Controller::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

		if (Ar.IsLoading())
		{
			
		}

		else
		{
			
		}
	}

#if WITH_EDITOR
	bool Controller::DrawDetailPanel()
	{
		bool Dirty = Actor::DrawDetailPanel();

		return Dirty;
	}
#endif
}