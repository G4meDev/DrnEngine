#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct AnimationPose
	{
		std::vector<Transform> BoneTransforms;

		static AnimationPose Blend(const AnimationPose& PoseA, const AnimationPose& PoseB, float Alpha);
	};
}