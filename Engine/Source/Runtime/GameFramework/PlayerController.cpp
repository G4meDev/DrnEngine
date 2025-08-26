#include "DrnPCH.h"
#include "PlayerController.h"

namespace Drn
{
	PlayerController::PlayerController()
		: Controller()
		, m_CameraManager(nullptr)
	{
		
		m_CameraManager = GetWorld()->SpawnActor<CameraManager>();
		m_CameraManager->SetActorLabel("CameraManager");
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

	void PlayerController::SetViewTarget( Actor* Target )
	{
		if (m_CameraManager)
		{
			m_CameraManager->SetViewTarget(Target);
		}
	}

}