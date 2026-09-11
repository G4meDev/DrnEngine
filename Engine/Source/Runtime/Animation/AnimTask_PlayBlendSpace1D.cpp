#include "DrnPCH.h"
#include "AnimTask_PlayBlendSpace1D.h"

namespace Drn
{
	float InterpSampleTime(float DeltaTime, float CurrentValue, float TargetValue, float InterplationSpeed, float RangeMin, float RangeMax, float Range)
	{
		if (CurrentValue == FLT_MAX || InterplationSpeed <= 0.0f)
		{
			CurrentValue = TargetValue;
		}
		else
		{
			CurrentValue = Math::FInterpConstantTo(CurrentValue, TargetValue, DeltaTime, InterplationSpeed * Range);
		}

		return Math::Clamp(CurrentValue, RangeMin, RangeMax);
	}

	void AnimTask_PlayBlendSpace1D::PlayBlendSpace1D( AssetHandle<BlendSpace1D> BlendSpace, float InSampleTime, float DeltaTime, float PlayRate)
	{
		BlendSpace1D* Animation = BlendSpace.Get();
		drn_check(Animation);

		if (!Animation->GetSkeleton().IsValid())
		{
			return;
		}

		const std::vector<BlendSpace1DSampleData>& AnimData = Animation->GetSampleData();
		const std::vector<uint8>& SortedSamples = Animation->GetSortedSampleIndices();
		const ReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetData().RefSkeleton;

		SampleTime = InterpSampleTime(DeltaTime, SampleTime, InSampleTime, Animation->GetInterplationSpeed(), Animation->GetRangeMin(), Animation->GetRangeMax(), Animation->GetRange());
		AnimationTimes.resize(SortedSamples.size(), 0.0f);

		struct SampleBlendData
		{
			const BlendSpace1DSampleData* AnimData = nullptr;
			float AnimationTime;
		};

		SampleBlendData SampleTargetA;
		SampleBlendData SampleTargetB;

		for (int32 SortedSampleIndex = 0; SortedSampleIndex < SortedSamples.size(); SortedSampleIndex++)
		{
			const uint8 SampleIndex = SortedSamples[SortedSampleIndex];
			const float SampleValue = AnimData[SampleIndex].SampleTime;

			if (SampleValue > SampleTime)
			{
				SampleTargetB.AnimData = &(AnimData[SampleIndex]);
				SampleTargetB.AnimationTime = AnimationTimes[SortedSampleIndex];

				if (SortedSampleIndex != 0)
				{
					int32 IndexA = SortedSamples[SortedSampleIndex-1];
					SampleTargetA.AnimData = &(AnimData[IndexA]);
					SampleTargetA.AnimationTime = AnimationTimes[SortedSampleIndex-1];
				}

				break;
			}

			if (SortedSampleIndex == SortedSamples.size() - 1)
			{
				int32 IndexA = SortedSamples[SortedSampleIndex];
				SampleTargetA.AnimData = &(AnimData[IndexA]);
				SampleTargetA.AnimationTime = AnimationTimes[SortedSampleIndex];
			}
		}

		if (SampleTargetA.AnimData != nullptr)
		{
			AnimationA.SetTime(SampleTargetA.AnimationTime);
			AnimationA.PlayAnimation(SampleTargetA.AnimData->Animation, DeltaTime, SampleTargetA.AnimData->PlayRate * PlayRate);
		}

		if (SampleTargetB.AnimData != nullptr)
		{
			AnimationB.SetTime(SampleTargetB.AnimationTime);
			AnimationB.PlayAnimation(SampleTargetB.AnimData->Animation, DeltaTime, SampleTargetB.AnimData->PlayRate * PlayRate);
		}

		if (!SampleTargetA.AnimData && !SampleTargetB.AnimData)
		{
			Pose.BoneTransforms.resize(RefSkeleton.BoneInfo.size());
		}

		else if (SampleTargetA.AnimData && !SampleTargetB.AnimData)
		{
			Pose = AnimationA.GetPose();
		}

		else if (!SampleTargetA.AnimData && SampleTargetB.AnimData)
		{
			Pose = AnimationB.GetPose();
		}

		else
		{
			float Alpha = Math::GetMappedRangeValueClamped(SampleTargetA.AnimData->SampleTime, SampleTargetB.AnimData->SampleTime, 0.0f, 1.0f, SampleTime);
			Pose = AnimationPose::Blend(AnimationA.GetPose(), AnimationB.GetPose(), Alpha);
		}

		// we keep separate sequence times. step each individually
		for (int32 SortedSampleIndex = 0; SortedSampleIndex < SortedSamples.size(); SortedSampleIndex++)
		{
			const uint8 SampleIndex = SortedSamples[SortedSampleIndex];
			const BlendSpace1DSampleData& Data = AnimData[SampleIndex];
			float& CurrentTime = AnimationTimes[SortedSampleIndex];

			CurrentTime = AnimationRuntime::StepAnimationTime(CurrentTime, DeltaTime, Data.Animation->GetAnimationData().Length, Data.PlayRate * PlayRate);
		}
	}

}