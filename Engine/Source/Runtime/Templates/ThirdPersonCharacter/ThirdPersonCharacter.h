#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ThirdPersonCharacter : public Character
	{
	public:
		ThirdPersonCharacter();
		virtual ~ThirdPersonCharacter();

		virtual void Serialize( Archive& Ar ) override;

		virtual EActorType GetActorType() override { return EActorType::ThirdPersonCharacter; }
		inline static EActorType GetActorTypeStatic() { return EActorType::ThirdPersonCharacter; };

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

		Rotator CameraRotation;

		std::shared_ptr<class SpringArmComponent> m_SpringArm;
		std::shared_ptr<class CameraComponent> m_Camera;
		std::shared_ptr<SkeletalMeshComponent> CharacterMesh;

		Vector m_ForwardInput;
		Vector m_RightInput;

		bool m_Running = false;

		float m_WalkSpeed = 10.0f;
		float m_RunSpeed = 27.0f;
		float m_LookSpeed = 70.0f;
		float m_CameraPitchClamp = 70.0f;
	};
}