#include "DrnPCH.h"
#include "Controller.h"

namespace Drn
{
	Controller::Controller()
		: Actor()
		, m_Pawn(nullptr)
	{
		SetTransient(true);

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
		// Nothing. Transient
	}

	void Controller::Possess( class Pawn* InPawn )
	{
		if (GetPawn() && InPawn != GetPawn())
		{
			UnPossess();
		}

		if (InPawn)
		{
			OnPossess(InPawn);
			m_Pawn = InPawn;
			InPawn->PossessBy(this);
		}
	}

	void Controller::UnPossess()
	{
		if (GetPawn())
		{
			OnUnPossess();
			GetPawn()->UnPossess();
			m_Pawn = nullptr;
		}
	}

	void Controller::OnPossess( class Pawn* InPawn )
	{
		
	}

	void Controller::OnUnPossess()
	{
		
	}

	void Controller::PostInitializeComponents()
	{
		Actor::PostInitializeComponents();


	}

#if WITH_EDITOR
	bool Controller::DrawDetailPanel()
	{
		bool Dirty = Actor::DrawDetailPanel();

		return Dirty;
	}
#endif
}