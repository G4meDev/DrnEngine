#include "DrnPCH.h"
#include "ThirdPersonCharacter.h"

#include "Editor/Misc/EditorMisc.h"
#include "Runtime/Components/InputComponent.h"
#include "Runtime/Components/SpringArmComponent.h"

#include "Runtime/Components/CharacterMovementComponent.h"

#if WITH_EDITOR
#include <Imgui.h>
#endif

namespace Drn
{
	ThirdPersonCharacter::ThirdPersonCharacter()
		: Character()
	{
		CharacterMesh = std::make_shared<SkeletalMeshComponent>();
		GetRoot()->AttachSceneComponent(CharacterMesh.get());
		CharacterMesh->SetComponentLabel("Character Mesh");

		m_SpringArm = std::make_shared<SpringArmComponent>();
		GetRoot()->AttachSceneComponent(m_SpringArm.get());
		m_SpringArm->SetComponentLabel("SpringArm");
		m_SpringArm->SetRelativeLocation(Vector(0.0f, 3.0f, 0.0f));

		m_Camera = std::make_shared<CameraComponent>();
		m_SpringArm->AttachSceneComponent(m_Camera.get());
		m_Camera->SetComponentLabel("Camera");


		AssetHandle<SkeletalMesh> MeshAsset("Engine\\Content\\Template\\ThirdPersonCharacter\\Character\\SK_ThirdPersonCharacter.drn");
		MeshAsset.Load();
		CharacterMesh->SetMesh(MeshAsset);

		CharacterAnimator = new ThirdPersonCharacterAnimator(this);
		CharacterMesh->SetAnimator(CharacterAnimator);

		GetCharacterMovementComponent()->SetHalfHeight(2.0f);
		GetCharacterMovementComponent()->SetRadius(0.7f);
	}

	ThirdPersonCharacter::~ThirdPersonCharacter()
	{
		
	}

	void ThirdPersonCharacter::Serialize( Archive& Ar )
	{
		Character::Serialize(Ar);

		m_SpringArm->Serialize(Ar);
		m_Camera->Serialize(Ar);
		CharacterMesh->Serialize(Ar);
	}

	void ThirdPersonCharacter::Tick( float DeltaTime )
	{
		Character::Tick(DeltaTime);

		const Vector VelocityZX = GetCharacterMovementComponent()->GetVelocity() * Vector(1.0f, 0.0f, 1.0f);
		const float VelocityZXMag = VelocityZX.Length();
		if (VelocityZXMag> KINDA_SMALL_NUMBER)
		{
			const float TurnRate = 10.0f;
			CharacterMesh->SetWorldRotation(Quat::Slerp(CharacterMesh->GetWorldRotation(), Quat::FromZ(VelocityZX), TurnRate * DeltaTime));
		}
		m_SpringArm->SetWorldRotation(CameraRotation.Quaternion());


		Vector ForwardVector = m_SpringArm->GetForwardVector() * Vector(1, 0, 1);
		ForwardVector = ForwardVector.GetSafeNormal();

		Vector RightVector = Vector::CrossProduct( Vector::UpVector, ForwardVector );

		m_MovementInput = ForwardVector * m_ForwardInput + RightVector * m_RightInput;
		if (m_MovementInput.Length() > 1.0f)
		{
			m_MovementInput = m_MovementInput.GetSafeNormal();
		}
		//m_MovementInput = m_MovementInput * (m_Running ? m_RunSpeed : m_WalkSpeed);

		{
			const float TargetSpeed = m_MovementInput.Length() * (m_Running ? m_RunSpeed : m_WalkSpeed);
			const float EffectiveSpeed = TargetSpeed > VelocityZXMag ? Math::FInterpTo(VelocityZXMag, TargetSpeed, DeltaTime, m_SpeedRaiseRate) : Math::FInterpTo(VelocityZXMag, TargetSpeed, DeltaTime, m_SpeedLowerRate);

			const Vector MovementDiretion = m_MovementInput.Length() > 0.001 ? m_MovementInput.GetSafeNormal() : VelocityZX.GetSafeNormal();
			m_MovementInput = MovementDiretion * EffectiveSpeed;
		}

		m_ForwardInput = m_RightInput = 0;
	}

	void ThirdPersonCharacter::CalcCamera( struct ViewInfo& OutResult )
	{
		m_Camera->GetCameraView(OutResult);
	}

	void ThirdPersonCharacter::SetupPlayerInputComponent( class InputComponent* PlayerInputComponent )
	{
		PlayerInputComponent->AddAxis(1, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnMoveForward);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyW, 1);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyS, -1);

		PlayerInputComponent->AddAxis(2, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnMoveRight);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyD, 1);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyA, -1);

		PlayerInputComponent->AddAnalog(3, this, &ThirdPersonCharacter::OnLookRight);
		PlayerInputComponent->AddAnalogMapping(3, gainput::MouseAxisX, -1);
		
		PlayerInputComponent->AddAnalog(4, this, &ThirdPersonCharacter::OnLookUp);
		PlayerInputComponent->AddAnalogMapping(4, gainput::MouseAxisY, 1);

		PlayerInputComponent->AddKey(5, this, &ThirdPersonCharacter::OnBeginRun, &ThirdPersonCharacter::OnEndRun);
		PlayerInputComponent->AddKeyMapping(5, gainput::KeyShiftL);

		PlayerInputComponent->AddAxis(6, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnLookUp);
		PlayerInputComponent->AddAxisMapping(6, gainput::KeyUp, 1);
		PlayerInputComponent->AddAxisMapping(6, gainput::KeyDown, -1);

		PlayerInputComponent->AddAxis(7, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnLookRight);
		PlayerInputComponent->AddAxisMapping(7, gainput::KeyRight, 1);
		PlayerInputComponent->AddAxisMapping(7, gainput::KeyLeft, -1);
	}

	void ThirdPersonCharacter::OnMoveForward( float Value )
	{
		m_ForwardInput = Value;
	}

	void ThirdPersonCharacter::OnMoveRight( float Value )
	{
		m_RightInput = Value;
	}

	void ThirdPersonCharacter::OnLookUp( float Value )
	{
		CameraRotation.Pitch += Time::GetApplicationDeltaTime() * -m_LookSpeed * Value;
		CameraRotation.Pitch = std::clamp(CameraRotation.Pitch, -m_CameraPitchClamp, m_CameraPitchClamp);
	}

	void ThirdPersonCharacter::OnLookRight( float Value )
	{
		CameraRotation.Yaw += Time::GetApplicationDeltaTime() * m_LookSpeed * Value;
	}

	void ThirdPersonCharacter::OnBeginRun()
	{
		m_Running = true;
	}

	void ThirdPersonCharacter::OnEndRun()
	{
		m_Running = false;
	}

#if WITH_EDITOR
	bool ThirdPersonCharacter::DrawDetailPanel()
	{
		bool Dirty = Character::DrawDetailPanel();

		return Dirty;
	}

	void ThirdPersonCharacter::DrawEditorDefault()
	{
		Character::DrawEditorDefault();

	}

	void ThirdPersonCharacter::DrawEditorSelected()
	{
		Character::DrawEditorSelected();

	}

#endif

// --------------------------------------------------------------------------------------

	ThirdPersonCharacterAnimator::ThirdPersonCharacterAnimator( ThirdPersonCharacter* InOwningCharacter )
		: OwningCharcater(InOwningCharacter)
	{
		const ReferenceSkeleton& RefSkeleton = InOwningCharacter->GetCharcaterMesh()->GetMesh()->GetData().RefSkeleton;
		const int32 BoneCount = RefSkeleton.BoneInfo.size();
		FinalPose.BoneTransforms.resize(BoneCount);

		IdleWalkRunBlendSpace = AssetHandle<BlendSpace1D>("Engine\\Content\\Template\\ThirdPersonCharacter\\Character\\Animation\\BS_ThirdPerson_IdleWalkRun.drn");
		IdleWalkRunBlendSpace.Load();

		//IdleAnimation = AssetHandle<AnimationSequence>("Engine\\Content\\Template\\ThirdPersonCharacter\\Character\\Animation\\AS_ThirdPerson_Idle.drn");
		//IdleAnimation.Load();
		//
		//WalkAnimation = AssetHandle<AnimationSequence>("Engine\\Content\\Template\\ThirdPersonCharacter\\Character\\Animation\\AS_ThirdPerson_Walk.drn");
		//WalkAnimation.Load();
		//
		//RunAnimation = AssetHandle<AnimationSequence>("Engine\\Content\\Template\\ThirdPersonCharacter\\Character\\Animation\\AS_ThirdPerson_Run.drn");
		//RunAnimation.Load();
	}

	bool TraceFootIK(Actor* OwningActor, const AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, int32 BoneIndex, float TraceDistance, Vector& HitLocation, Vector& HitNormal, float& HitOffset)
	{
		const int32 JointIndex = RefSkeleton.BoneInfo[BoneIndex].ParentIndex;
		const int32 RootIndex = RefSkeleton.BoneInfo[JointIndex].ParentIndex;

		const Vector BoneWorldLocation = (Pose.BoneTransforms[BoneIndex] * ComponentTransform).GetLocation();
		const Vector FlattenFootLocation = Vector(BoneWorldLocation.X, ComponentTransform.GetLocation().Y, BoneWorldLocation.Z);
		const Vector TraceStart = FlattenFootLocation + Vector::UpVector * TraceDistance;
		const Vector TraceEnd = FlattenFootLocation + Vector::DownVector * TraceDistance;

		HitResult Hit;
		bool bHit = OwningActor->GetWorld()->LineTrace(Hit, TraceStart, TraceEnd, {ECollisionChannel::ECC_WorldStatic}, {OwningActor}, 0.0f);

		if (bHit)
		{
			HitLocation = Hit.Location;
			HitNormal = Hit.Normal;
			HitOffset = HitLocation.Y - FlattenFootLocation.Y;
		}
		else
		{
			HitLocation = FlattenFootLocation;
			HitNormal = Vector::UpVector;
			HitOffset = 0.0f;
		}

		return bHit;
	}

	Quat CalculateFootRotation(const Vector& Normal)
	{
		return Rotator(Math::RadiansToDegrees(Math::Atan2(Normal.Z, Normal.Y)), 0.0f,
			Math::RadiansToDegrees(Math::Atan2(Normal.X, Normal.Y))).Quaternion();
	}

	void ThirdPersonCharacterAnimator::Tick( float DeltaTime )
	{
		Animator::Tick(DeltaTime);
		OwningComponent->MarkRenderStateDirty();

		float CharacterSpeed = OwningCharcater->GetCharacterMovementComponent()->GetVelocity().LengthXZ();

		const ReferenceSkeleton& RefSkeleton = OwningComponent->GetMesh()->GetData().RefSkeleton;
		const int32 BoneCount = RefSkeleton.BoneInfo.size();

		PlayIdleWalkRun.PlayBlendSpace1D(IdleWalkRunBlendSpace, CharacterSpeed, DeltaTime);

		//PlayAnimationIdle.PlayAnimation(IdleAnimation, DeltaTime);
		//PlayAnimationWalk.PlayAnimation(WalkAnimation, DeltaTime);
		//PlayAnimationRun.PlayAnimation(RunAnimation, DeltaTime);
		//
		//const int32 BoneCount = RefSkeleton.BoneInfo.size();
		//FinalPose.BoneTransforms.resize(BoneCount);
		//
		//if (CharacterSpeed < OwningCharcater->GetWalkSpeed())
		//{
		//	float Alpha = Math::GetMappedRangeValueClamped(0.0f, OwningCharcater->GetWalkSpeed(), 0.0f, 1.0f, LerpedSpeed);
		//	FinalPose = AnimationPose::Blend(PlayAnimationIdle.GetPose(), PlayAnimationWalk.GetPose(), Alpha);
		//}
		//else
		//{
		//	float Alpha = Math::GetMappedRangeValueClamped(OwningCharcater->GetWalkSpeed(), OwningCharcater->GetRunSpeed(), 0.0f, 1.0f, LerpedSpeed);
		//	FinalPose = AnimationPose::Blend(PlayAnimationWalk.GetPose(), PlayAnimationRun.GetPose(), Alpha);
		//}

		FinalPose = PlayIdleWalkRun.GetPose();

		Vector IKLocation_RF; Vector IKNormal_RF; float IKOffset_RF;
		Vector IKLocation_LF; Vector IKNormal_LF; float IKOffset_LF;

		const std::string BoneName_RF = "mixamorig:RightFoot";
		const int32 BoneIndex_RF = RefSkeleton.FindBone(BoneName_RF);
		drn_check(BoneIndex_RF >= 0);

		const std::string BoneName_LF = "mixamorig:LeftFoot";
		const int32 BoneIndex_LF = RefSkeleton.FindBone(BoneName_LF);
		drn_check(BoneIndex_LF >= 0);

		const float FootTraceDistance = 1.0f;
		const Transform ComponentTransform = OwningComponent->GetWorldTransform();
		TraceFootIK(OwningCharcater, FinalPose, RefSkeleton, ComponentTransform, BoneIndex_RF, FootTraceDistance, IKLocation_RF, IKNormal_RF, IKOffset_RF);
		TraceFootIK(OwningCharcater, FinalPose, RefSkeleton, ComponentTransform, BoneIndex_LF, FootTraceDistance, IKLocation_LF, IKNormal_LF, IKOffset_LF);

		const std::string BoneName_Root = "mixamorig:Hips";
		const int32 BoneIndex_Root = RefSkeleton.FindBone(BoneName_Root);
		drn_check(BoneIndex_Root >= 0);

		const Vector RootOffset = Vector::UpVector * (IKOffset_RF + IKOffset_LF) * 0.5f;
		AnimationRuntime::ModifyBoneTransform(FinalPose, RefSkeleton, ComponentTransform, BoneIndex_Root, Transform(RootOffset),
			EBoneControlSpace::WorldSpace, EBoneModificationMode::Ignore, EBoneModificationMode::Additive, EBoneModificationMode::Ignore);

		const Vector JointTarget_RF = Vector(0.5f, 1.5f, 2.0f);
		const Vector JointTarget_LF = Vector(-0.5f, 1.5f, 2.0f);

		//OwningCharcater->GetWorld()->DrawDebugSphere(ComponentTransform.TransformPosition(JointTarget_RF), Quat::Identity, Color::Red, 1.0f, 32, 0.0, 0.0f);

		const Vector Effector_RF = IKLocation_RF + Vector::UpVector * FinalPose.BoneTransforms[BoneIndex_RF].GetLocation().Y;
		const Vector Effector_LF = IKLocation_LF + Vector::UpVector * FinalPose.BoneTransforms[BoneIndex_LF].GetLocation().Y;

		AnimationRuntime::TwoBoneIK(FinalPose, RefSkeleton, ComponentTransform, BoneIndex_RF, JointTarget_RF, Effector_RF,
			false, 1.0f, 1.0f, EBoneControlSpace::ComponentSpace, EBoneControlSpace::WorldSpace);

		AnimationRuntime::TwoBoneIK(FinalPose, RefSkeleton, ComponentTransform, BoneIndex_LF, JointTarget_LF, Effector_LF,
			false, 1.0f, 1.0f, EBoneControlSpace::ComponentSpace, EBoneControlSpace::WorldSpace);

		AnimationRuntime::ModifyBoneTransform(FinalPose, RefSkeleton, ComponentTransform, BoneIndex_RF, Transform(Vector::ZeroVector, CalculateFootRotation(IKNormal_RF)),
			EBoneControlSpace::WorldSpace, EBoneModificationMode::Ignore, EBoneModificationMode::Additive, EBoneModificationMode::Ignore);

		AnimationRuntime::ModifyBoneTransform(FinalPose, RefSkeleton, ComponentTransform, BoneIndex_LF, Transform(Vector::ZeroVector, CalculateFootRotation(IKNormal_LF)),
			EBoneControlSpace::WorldSpace, EBoneModificationMode::Ignore, EBoneModificationMode::Additive, EBoneModificationMode::Ignore);

		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			FinalPose.BoneTransforms[BoneIndex] = RefSkeleton.BonePose[BoneIndex] * FinalPose.BoneTransforms[BoneIndex];
		}
	}

	Matrix ThirdPersonCharacterAnimator::GetFinalBoneMatrix( int32 BoneIndex ) const
	{
		drn_check(BoneIndex >= 0);
		drn_check(BoneIndex < FinalPose.BoneTransforms.size());

		return FinalPose.BoneTransforms[BoneIndex];
	}

	int32 ThirdPersonCharacterAnimator::GetBoneCount() const
	{
		return FinalPose.BoneTransforms.size();
	}

}  // namespace Drn