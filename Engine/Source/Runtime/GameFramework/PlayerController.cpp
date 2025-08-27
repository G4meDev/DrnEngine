#include "DrnPCH.h"
#include "PlayerController.h"

namespace Drn
{
	PlayerController::PlayerController()
		: Controller()
		, m_CameraManager(nullptr)
	{

	}

	PlayerController::~PlayerController()
	{
		
	}

	void PlayerController::Tick( float DeltaTime )
	{
		Controller::Tick(DeltaTime);


	}

	void PlayerController::OnPossess( class Pawn* InPawn )
	{
		Controller::OnPossess(InPawn);

		SetViewTarget(InPawn);
	}

	void PlayerController::OnUnPossess()
	{
		Controller::OnUnPossess();


	}

	void PlayerController::PostInitializeComponents()
	{
		Controller::PostInitializeComponents();

		m_CameraManager = GetWorld()->SpawnActor<CameraManager>();
		m_CameraManager->SetActorLabel("CameraManager");

		SetViewTarget(m_Pawn);
	}

	void PlayerController::SetViewTarget( Actor* Target )
	{
		if (m_CameraManager)
		{
			m_CameraManager->SetViewTarget(Target);
		}
	}

}