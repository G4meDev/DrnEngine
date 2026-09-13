#pragma once

#include "ForwardTypes.h"
#include "Runtime/Animation/AnimationPose.h"

namespace Drn
{
	class AnimationRuntime
	{
	public:
		static void GetFrameIndicesFromTime(int32& OutKeyIndex1, int32& OutKeyIndex2, float& OutAlpha, const float Time, const int32 NumFrames, const float SequenceLength);
		static float StepAnimationTime(float CurrentTime, float DeltaTime, float AnimationLength, float PlayRate = 1.0f, bool bLoop = true);

		static void SolveTwoBoneIK( const Vector& RootPos, const Vector& JointPos, const Vector& EndPos, const Vector& JointTarget,const Vector& Effector, Vector& OutJointPos,
			Vector& OutEndPos, float UpperLimbLength, float LowerLimbLength, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale );

		static void SolveTwoBoneIK(const Vector& RootPos, const Vector& JointPos, const Vector& EndPos, const Vector& JointTarget, const Vector& Effector, Vector& OutJointPos,
			Vector& OutEndPos, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale);

		static void SolveTwoBoneIK(Transform& InOutRootTransform, Transform& InOutJointTransform, Transform& InOutEndTransform, const Vector& JointTarget, const Vector& Effector,
			float UpperLimbLength, float LowerLimbLength, bool bAllowStretching, float StartStretchRatio, float MaxStretchScale);

		static void SolveTwoBoneIK(Transform& InOutRootTransform, Transform& InOutJointTransform, Transform& InOutEndTransform, const Vector& JointTarget, const Vector& Effector,
			bool bAllowStretching, float StartStretchRatio, float MaxStretchScale);

		static void TwoBoneIK(AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, int32 BoneIndex, const Vector& JointTarget, const Vector& Effector,
			bool bAllowStretching, float StartStretchRatio, float MaxStretchScale, EBoneControlSpace JointSpace = EBoneControlSpace::WorldSpace, EBoneControlSpace EffectorSpace = EBoneControlSpace::WorldSpace);

		static void ConvertFromComponentSpace(AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, Transform& InOutTransform, int32 BoneIndex, EBoneControlSpace Space);
		static void ConvertToComponentSpace(AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, Transform& InOutTransform, int32 BoneIndex, EBoneControlSpace Space);

		static void ModifyBoneTransform(AnimationPose& Pose, const ReferenceSkeleton& RefSkeleton, const Transform& ComponentTransform, int32 BoneIndex, const Transform& BoneTransform,
			EBoneControlSpace Space, EBoneModificationMode TranslationMode = EBoneModificationMode::Ignore,
			EBoneModificationMode RotationMode = EBoneModificationMode::Ignore, EBoneModificationMode ScaleMode = EBoneModificationMode::Ignore);
	};
}