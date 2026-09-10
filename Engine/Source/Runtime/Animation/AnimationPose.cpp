#include "DrnPCH.h"
#include "AnimationPose.h"

namespace Drn
{
	AnimationPose AnimationPose::Blend( const AnimationPose& PoseA, const AnimationPose& PoseB, float Alpha )
	{
		drn_check(PoseA.BoneTransforms.size() == PoseB.BoneTransforms.size());

		const int32 BoneCount = PoseA.BoneTransforms.size();
		AnimationPose Result;
		Result.BoneTransforms.reserve(BoneCount);

		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			Result.BoneTransforms.push_back(Transform::Blend(PoseA.BoneTransforms[BoneIndex], PoseB.BoneTransforms[BoneIndex], Alpha));
		}

		return Result;
	}
}