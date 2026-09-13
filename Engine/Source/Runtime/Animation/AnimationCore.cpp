#include "DrnPCH.h"
#include "AnimationCore.h"

namespace Drn
{
	void AnimationRuntime::GetFrameIndicesFromTime( int32& OutKeyIndex1, int32& OutKeyIndex2, float& OutAlpha, const float Time, const int32 NumFrames, const float SequenceLength )
	{
		if( Time <= 0.f || NumFrames == 1 )
		{
			OutKeyIndex1 = 0;
			OutKeyIndex2 = 0;
			OutAlpha = 0.f;
			return;
		}

		const int32 LastIndex = NumFrames - 1;
		if( Time >= SequenceLength )
		{
			OutKeyIndex1 = LastIndex;
			OutKeyIndex2 = (OutKeyIndex1 + 1) % (NumFrames);
			OutAlpha = 0.f;
			return;
		}

		const int32 NumKeys = NumFrames - 1;
		const float KeyPos = ((float)NumKeys * Time) / SequenceLength;

		const int32 KeyIndex1 = std::clamp<int32>( Math::FloorToInt(KeyPos), 0, NumFrames-1 );
		const float Alpha = KeyPos - (float)KeyIndex1;

		int32 KeyIndex2 = KeyIndex1 + 1;
		if( KeyIndex2 == NumFrames )
		{
			KeyIndex2 = KeyIndex1;
		}

		OutKeyIndex1 = KeyIndex1;
		OutKeyIndex2 = KeyIndex2;
		OutAlpha = Alpha;
	}

	float AnimationRuntime::StepAnimationTime(float CurrentTime, float DeltaTime, float AnimationLength, float PlayRate, bool bLoop)
	{
		CurrentTime += DeltaTime * PlayRate;
		if (bLoop)
		{
			return std::fmod(CurrentTime, AnimationLength);
		}
		else
		{
			return std::min(CurrentTime, AnimationLength);
		}
	}

	void AnimationRuntime::SolveTwoBoneIK( const Vector& RootPos, const Vector& JointPos, const Vector& EndPos, const Vector& JointTarget, const Vector& Effector, Vector& OutJointPos,
		Vector& OutEndPos, float UpperLimbLength, float LowerLimbLength, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale )
	{
		SCOPE_STAT();

		Vector DesiredPos = Effector;
		Vector DesiredDelta = DesiredPos - RootPos;
		float DesiredLength = DesiredDelta.Length();

		float MaxLimbLength = LowerLimbLength + UpperLimbLength;

		Vector	DesiredDir;
		if (DesiredLength < (float)KINDA_SMALL_NUMBER)
		{
			DesiredLength = (float)KINDA_SMALL_NUMBER;
			DesiredDir = Vector(0, 0, 1);
		}
		else
		{
			DesiredDir = DesiredDelta.GetSafeNormal();
		}

		Vector JointTargetDelta = JointTarget - RootPos;
		const float JointTargetLengthSqr = JointTargetDelta.SizeSquared();

		Vector JointPlaneNormal, JointBendDir;
		if (JointTargetLengthSqr < Math::Square((float)KINDA_SMALL_NUMBER))
		{
			JointBendDir = Vector(0, 0, 1);
			JointPlaneNormal = Vector(0, 1, 0);
		}
		else
		{
			JointPlaneNormal = DesiredDir ^ JointTargetDelta;

			if (JointPlaneNormal.SizeSquared() < Math::Square((float)KINDA_SMALL_NUMBER))
			{
				DesiredDir.FindBestAxisVectors(JointPlaneNormal, JointBendDir);
			}
			else
			{
				JointPlaneNormal.Normalize();

				JointBendDir = JointTargetDelta - ( DesiredDir * ( JointTargetDelta | DesiredDir ) );
				JointBendDir.Normalize();
			}
		}

		if (bAllowStretching)
		{
			const float ScaleRange = MaxStretchScale - StartStretchRatio;
			if (ScaleRange > KINDA_SMALL_NUMBER && MaxLimbLength > KINDA_SMALL_NUMBER)
			{
				const float ReachRatio = DesiredLength / MaxLimbLength;
				const float ScalingFactor = (MaxStretchScale - 1.f) * Math::Clamp<float>((ReachRatio - StartStretchRatio) / ScaleRange, 0.f, 1.f);
				if (ScalingFactor > KINDA_SMALL_NUMBER)
				{
					LowerLimbLength *= (1.f + ScalingFactor);
					UpperLimbLength *= (1.f + ScalingFactor);
					MaxLimbLength *= (1.f + ScalingFactor);
				}
			}
		}

		OutEndPos = DesiredPos;
		OutJointPos = JointPos;

		if (DesiredLength >= MaxLimbLength)
		{
			OutEndPos = RootPos + (DesiredDir * MaxLimbLength);
			OutJointPos = RootPos + (DesiredDir * UpperLimbLength);
		}
		else
		{
			const float TwoAB = 2.f * UpperLimbLength * DesiredLength;
			const float CosAngle = (TwoAB != 0.f) ? ((UpperLimbLength*UpperLimbLength) + (DesiredLength*DesiredLength) - (LowerLimbLength*LowerLimbLength)) / TwoAB : 0.f;
			const bool bReverseUpperBone = (CosAngle < 0.f);
			const float Angle = Math::Acos(CosAngle);

			const float JointLineDist = UpperLimbLength * Math::Sin(Angle);

			const float ProjJointDistSqr = (UpperLimbLength*UpperLimbLength) - (JointLineDist*JointLineDist);
			float ProjJointDist = (ProjJointDistSqr > 0.f) ? Math::Sqrt(ProjJointDistSqr) : 0.f;
			if (bReverseUpperBone)
			{
				ProjJointDist *= -1.f;
			}

			OutJointPos = RootPos + (DesiredDir * ProjJointDist) + (JointBendDir * JointLineDist);
		}
	}

	void AnimationRuntime::SolveTwoBoneIK( const Vector& RootPos, const Vector& JointPos, const Vector& EndPos, const Vector& JointTarget, const Vector& Effector, Vector& OutJointPos,
		Vector& OutEndPos, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale )
	{
		float LowerLimbLength = (EndPos - JointPos).Length();
		float UpperLimbLength = (JointPos - RootPos).Length();

		SolveTwoBoneIK(RootPos, JointPos, EndPos, JointTarget, Effector, OutJointPos, OutEndPos, UpperLimbLength, LowerLimbLength, bAllowStretching, StartStretchRatio, MaxStretchScale);
	}

	void AnimationRuntime::SolveTwoBoneIK( Transform& InOutRootTransform, Transform& InOutJointTransform, Transform& InOutEndTransform, const Vector& JointTarget,
		const Vector& Effector, float UpperLimbLength, float LowerLimbLength, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale )
	{
		Vector OutJointPos, OutEndPos;

		Vector RootPos = InOutRootTransform.GetLocation();
		Vector JointPos = InOutJointTransform.GetLocation();
		Vector EndPos = InOutEndTransform.GetLocation();

		SolveTwoBoneIK(RootPos, JointPos, EndPos, JointTarget, Effector, OutJointPos, OutEndPos, UpperLimbLength, LowerLimbLength, bAllowStretching, StartStretchRatio, MaxStretchScale);

		{
			Vector const OldDir = (JointPos - RootPos).GetSafeNormal();
			Vector const NewDir = (OutJointPos - RootPos).GetSafeNormal();

			Quat const DeltaRotation = Quat::FindBetweenNormals(OldDir, NewDir);
			InOutRootTransform.SetRotation(DeltaRotation * InOutRootTransform.GetRotation());
			InOutRootTransform.SetLocation(RootPos);
		}

		{
			Vector const OldDir = (EndPos - JointPos).GetSafeNormal();
			Vector const NewDir = (OutEndPos - OutJointPos).GetSafeNormal();

			Quat const DeltaRotation = Quat::FindBetweenNormals(OldDir, NewDir);
			InOutJointTransform.SetRotation(DeltaRotation * InOutJointTransform.GetRotation());
			InOutJointTransform.SetLocation(OutJointPos);
		}

		InOutEndTransform.SetLocation(OutEndPos);
	}

	void AnimationRuntime::SolveTwoBoneIK( Transform& InOutRootTransform, Transform& InOutJointTransform, Transform& InOutEndTransform, const Vector& JointTarget,
		const Vector& Effector, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale )
	{
		float LowerLimbLength = (InOutEndTransform.GetLocation() - InOutJointTransform.GetLocation()).Length();
		float UpperLimbLength = (InOutJointTransform.GetLocation() - InOutRootTransform.GetLocation()).Length();
		SolveTwoBoneIK(InOutRootTransform, InOutJointTransform, InOutEndTransform, JointTarget, Effector, UpperLimbLength, LowerLimbLength, bAllowStretching, StartStretchRatio, MaxStretchScale);
	}

	void AnimationRuntime::TwoBoneIK( AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, int32 BoneIndex, const Vector& JointTarget,
		const Vector& Effector, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale, EBoneControlSpace JointSpace, EBoneControlSpace EffectorSpace )
	{
		drn_check(BoneIndex >= 0);

		int32 JointIndex = RefSkeleton.BoneInfo[BoneIndex].ParentIndex;
		drn_check(JointIndex >= 0);

		int32 RootIndex = RefSkeleton.BoneInfo[JointIndex].ParentIndex;
		drn_check(RootIndex >= 0);

		Transform RootTransform = Pose.BoneTransforms[RootIndex];
		Transform JointTransform = Pose.BoneTransforms[JointIndex];
		Transform EndTransform = Pose.BoneTransforms[BoneIndex];

		Transform EffectorTargetTransform(Effector, Quat::Identity);
		Transform JointTargetTransform(JointTarget, Quat::Identity);

		ConvertToComponentSpace(Pose, RefSkeleton, ComponentTransform, EffectorTargetTransform, BoneIndex, EffectorSpace);
		ConvertToComponentSpace(Pose, RefSkeleton, ComponentTransform, JointTargetTransform, JointIndex, JointSpace);

		SolveTwoBoneIK(RootTransform, JointTransform, EndTransform, JointTargetTransform.GetLocation(), EffectorTargetTransform.GetLocation(), bAllowStretching, StartStretchRatio, MaxStretchScale);

		AnimationRuntime::ModifyBoneTransform(Pose, RefSkeleton, ComponentTransform, RootIndex, RootTransform,
			EBoneControlSpace::ComponentSpace, EBoneModificationMode::Replace, EBoneModificationMode::Replace, EBoneModificationMode::Replace);

		AnimationRuntime::ModifyBoneTransform(Pose, RefSkeleton, ComponentTransform, JointIndex, JointTransform,
			EBoneControlSpace::ComponentSpace, EBoneModificationMode::Replace, EBoneModificationMode::Replace, EBoneModificationMode::Replace);

		AnimationRuntime::ModifyBoneTransform(Pose, RefSkeleton, ComponentTransform, BoneIndex, EndTransform,
			EBoneControlSpace::ComponentSpace, EBoneModificationMode::Replace, EBoneModificationMode::Replace, EBoneModificationMode::Replace);
	}

	void AnimationRuntime::ConvertFromComponentSpace( AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, Transform& InOutTransform, int32 BoneIndex, EBoneControlSpace Space )
	{
		if (Space == EBoneControlSpace::ComponentSpace)
		{
			// nothing
		}

		else if (Space == EBoneControlSpace::WorldSpace)
		{
			InOutTransform = InOutTransform * ComponentTransform;
		}

		else
		{
			drn_check(false);
		}
	}

	void AnimationRuntime::ConvertToComponentSpace( AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, Transform& InOutTransform, int32 BoneIndex, EBoneControlSpace Space )
	{
		if (Space == EBoneControlSpace::ComponentSpace)
		{
			// nothing
		}

		else if (Space == EBoneControlSpace::WorldSpace)
		{
			InOutTransform = InOutTransform.GetRelativeTransform(ComponentTransform);
		}

		else
		{
			drn_check(false);
		}
	}

	void AnimationRuntime::ModifyBoneTransform( AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, int32 BoneIndex, const Transform& BoneTransform,
		EBoneControlSpace Space, EBoneModificationMode TranslationMode, EBoneModificationMode RotationMode, EBoneModificationMode ScaleMode)
	{
		SCOPE_STAT();
		drn_check(BoneIndex >= 0);

		Transform NewTransform = Pose.BoneTransforms[BoneIndex];
		ConvertFromComponentSpace(Pose, RefSkeleton, ComponentTransform, NewTransform, BoneIndex, Space);

		if (ScaleMode != EBoneModificationMode::Ignore)
		{
			if (ScaleMode == EBoneModificationMode::Additive)
			{
				NewTransform.SetScale(NewTransform.GetScale() * BoneTransform.GetScale());
			}
			else
			{
				NewTransform.SetScale(BoneTransform.GetScale());
			}
		}

		if (RotationMode != EBoneModificationMode::Ignore)
		{
			if (RotationMode == EBoneModificationMode::Additive)
			{
				NewTransform.SetRotation(BoneTransform.GetRotation() * NewTransform.GetRotation());
			}
			else
			{
				NewTransform.SetRotation(BoneTransform.GetRotation());
			}
		}

		if (TranslationMode != EBoneModificationMode::Ignore)
		{
			if (TranslationMode == EBoneModificationMode::Additive)
			{
				NewTransform.SetLocation(NewTransform.GetLocation() + BoneTransform.GetLocation());
			}
			else
			{
				NewTransform.SetLocation(BoneTransform.GetLocation());
			}
		}

		ConvertToComponentSpace(Pose, RefSkeleton, ComponentTransform, NewTransform, BoneIndex, Space);

		struct ChildParentTransform
		{
			Transform BoneTransform;
			int32 BoneIndex;
		};
		std::vector<ChildParentTransform> CachedTransforms;

		int32 ParentIndex = BoneIndex;
		for (int32 ChildIndex = 0; ChildIndex < RefSkeleton.BoneInfo.size(); ChildIndex++)
		{
			const auto& Child = RefSkeleton.BoneInfo[ChildIndex];

			if (Child.ParentIndex == ParentIndex)
			{
				const Transform& ParentTransform = Pose.BoneTransforms[ParentIndex];
				const Transform& ChildTransform = Pose.BoneTransforms[ChildIndex];

				CachedTransforms.push_back({});
				CachedTransforms.back().BoneIndex = ChildIndex;
				CachedTransforms.back().BoneTransform = ChildTransform.GetRelativeTransform(ParentTransform);

				ParentIndex = ChildIndex;
			}
		}

		Pose.BoneTransforms[BoneIndex] = NewTransform;
		for (const ChildParentTransform& T : CachedTransforms)
		{
			const Transform& ParentTransform = Pose.BoneTransforms[RefSkeleton.BoneInfo[T.BoneIndex].ParentIndex];
			Pose.BoneTransforms[T.BoneIndex] = T.BoneTransform * ParentTransform;
		}
	}

        }  // namespace Drn