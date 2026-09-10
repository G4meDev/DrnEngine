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

		inline SkeletalMeshComponent* GetCharcaterMesh() const { return CharacterMesh.get(); }

		virtual void Tick( float DeltaTime ) override;

		virtual void CalcCamera( struct ViewInfo& OutResult ) override;

		void SetupPlayerInputComponent( class InputComponent* PlayerInputComponent ) override;

		void OnMoveForward( float Value );
		void OnMoveRight(float Value);

		void OnLookUp(float Value);
		void OnLookRight(float Value);

		void OnBeginRun();
		void OnEndRun();

		inline float GetWalkSpeed() const { return m_WalkSpeed; }
		inline float GetRunSpeed() const { return m_RunSpeed; }

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

		TRefCountPtr<class ThirdPersonCharacterAnimator> CharacterAnimator;

		Vector m_ForwardInput;
		Vector m_RightInput;

		bool m_Running = false;

		float m_WalkSpeed = 4.0f;
		float m_RunSpeed = 10.0f;
		float m_SpeedRaiseRate = 20.0f;
		float m_SpeedLowerRate = 50.0f;

		float m_LookSpeed = 70.0f;
		float m_CameraPitchClamp = 70.0f;
	};

	class ThirdPersonCharacterAnimator : public Animator
	{
	public:
		ThirdPersonCharacterAnimator(ThirdPersonCharacter* InOwningCharacter);

		virtual void Tick(float DeltaTime) override;

		virtual Matrix GetFinalBoneMatrix(int32 BoneIndex) const override;
		virtual int32 GetBoneCount() const override;

		AnimTask_PlayAnimation PlayAnimationIdle;
		AnimTask_PlayAnimation PlayAnimationWalk;
		AnimTask_PlayAnimation PlayAnimationRun;

		ThirdPersonCharacter* OwningCharcater;

		AssetHandle<AnimationSequence> IdleAnimation;
		AssetHandle<AnimationSequence> WalkAnimation;
		AssetHandle<AnimationSequence> RunAnimation;

		AnimationPose FinalPose;

		float LerpedSpeed = 0.0f;
	};
}