#pragma once

#include "ForwardTypes.h"
#include "Runtime/Animation/AnimationPose.h"
#include "Runtime/Animation/AnimTask_PlayAnimation.h"

namespace Drn
{
	class AnimTask_PlayBlendSpace1D
	{
	public:
		void PlayBlendSpace1D(AssetHandle<BlendSpace1D> BlendSpace, float InSampleTime, float DeltaTime, float PlayRate = 1.0f);

		inline const AnimationPose GetPose() const { return Pose; }

	protected:
		float SampleTime = FLT_MAX;

		AnimTask_PlayAnimation AnimationA;
		AnimTask_PlayAnimation AnimationB;

		std::vector<float> AnimationTimes;

		AnimationPose Pose;
	};
}