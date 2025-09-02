#pragma once

#include "ForwardTypes.h"
#include "Runtime/GameFramework/Character.h"
#include "GameForwardTypes.h"

namespace Drn
{
	class TestPlayerCharacter : public Character
	{
	public:
		TestPlayerCharacter();
		virtual ~TestPlayerCharacter();

		virtual void Serialize( Archive& Ar ) override;

		virtual EActorType GetActorType() override { return static_cast<EActorType>(EGameActorType::TestPlayerCharacter); }
		inline static EActorType GetActorTypeStatic() { return static_cast<EActorType>(EGameActorType::TestPlayerCharacter); };

		virtual void Tick( float DeltaTime ) override;

		virtual void CalcCamera( struct ViewInfo& OutResult ) override;

		void SetupPlayerInputComponent( class InputComponent* PlayerInputComponent ) override;

		void OnMoveForward( float Value );
		void OnMoveRight(float Value);

		void OnLookUp(float Value);
		void OnLookRight(float Value);

		void OnBeginRun();
		void OnEndRun();

#if WITH_EDITOR
		virtual bool DrawDetailPanel() override;
		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
#endif

	protected:

		std::shared_ptr<class SpringArmComponent> m_SpringArm;
		std::shared_ptr<class CameraComponent> m_Camera;

		Vector m_ForwardInput;
		Vector m_RightInput;

		bool m_Running = false;

		float m_WalkSpeed = 10.0f;
		float m_RunSpeed = 27.0f;
	};
}