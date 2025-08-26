#pragma once

#include "ForwardTypes.h"
#include "Runtime/GameFramework/Controller.h"

namespace Drn
{
	class PlayerController : public Controller
	{
	public:
		PlayerController();
		virtual ~PlayerController();

		void Tick( float DeltaTime ) override;
		EActorType GetActorType() override { return EActorType::PlayerController; };

		inline class CameraManager* GetCameraManager() const { return m_CameraManager; }

		void OnPossess( class Pawn* InPawn ) override;
		void OnUnPossess() override;

		void SetViewTarget( Actor* Target );

 protected:
		class CameraManager* m_CameraManager;
	};
}