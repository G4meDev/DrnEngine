#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class AnimTask_PlayAnimation
	{
	public:
		void PlayAnimation(AssetHandle<AnimationSequence> Sequence, float DeltaTime, float Rate = 1.0f, bool bLoop = true);
		void SetTime(float Time) { AnimTime = Time; }
		void Reset() { SetTime(0); }

		inline float GetRemainingTime() const { return AnimLength - AnimTime; };
		inline float GetRemainingTimeRatio() const { return (AnimLength - AnimTime) / AnimLength; };

		inline const AnimationPose& GetPose() const { return Pose; }

	protected:
		float AnimTime;
		float AnimLength;
		AnimationPose Pose;
	};
}