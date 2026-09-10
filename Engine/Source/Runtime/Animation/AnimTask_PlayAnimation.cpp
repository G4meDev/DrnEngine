#include "DrnPCH.h"
#include "AnimTask_PlayAnimation.h"

namespace Drn
{
	void AnimTask_PlayAnimation::PlayAnimation( AssetHandle<AnimationSequence> Sequence, float DeltaTime, float Rate, bool bLoop )
	{
		AnimationSequence* Animation = Sequence.Get();
		drn_check(Animation);
		drn_check(Animation->GetSkeleton().IsValid());

		const AnimationData& AnimData = Animation->GetAnimationData();
		const ReferenceSkeleton& RefSkeleton = Sequence->GetSkeleton()->GetData().RefSkeleton;

		AnimTime = AnimTime + DeltaTime * Rate;
		if (bLoop)
		{
			AnimTime = std::fmod(AnimTime, AnimData.Length);
		}
		else
		{
			AnimTime = std::min(AnimTime, AnimData.Length);
		}

		int32 FrameIndex1; int32 FrameIndex2; float Alpha;
		AnimationRuntime::GetFrameIndicesFromTime(FrameIndex1, FrameIndex2, Alpha, AnimTime, Animation->GetNumFrames(), AnimData.Length);

		const AnimationKeyFrame& Frame1 = AnimData.KeyFrames[FrameIndex1];
		const AnimationKeyFrame& Frame2 = AnimData.KeyFrames[FrameIndex2];

		const int32 BoneCount = RefSkeleton.BoneInfo.size();
		Pose.BoneTransforms.resize(BoneCount);

		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			const int32 ParentBoneIndex = RefSkeleton.BoneInfo[BoneIndex].ParentIndex;
			Transform BoneTransform = Transform::Blend(Frame1.BonePose[BoneIndex], Frame2.BonePose[BoneIndex], Alpha);

			if (ParentBoneIndex == -1)
			{
				Pose.BoneTransforms[BoneIndex] = BoneTransform;
			}
			else
			{
				Pose.BoneTransforms[BoneIndex] = BoneTransform * Pose.BoneTransforms[ParentBoneIndex];
			}	
		}
	}
}