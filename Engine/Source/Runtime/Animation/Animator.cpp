#include "DrnPCH.h"
#include "Animator.h"

#include "Editor/AssetPreview/AssetPreviewSkeletalMeshGuiLayer.h"
#include "Editor/AssetPreview/AssetPreviewAnimationSequenceGuiLayer.h"

namespace Drn
{
	void Animator::Tick( float DeltaTime )
	{
	}

	void AnimatorReferencePose::Tick( float DeltaTime )
	{
		
	}

	const Matrix& AnimatorReferencePose::GetFinalBoneMatrix(int32 BoneIndex) const
	{
		return Matrix::MatrixIdentity;
	}
	
	int32 AnimatorReferencePose::GetBoneCount() const
	{
		drn_check(OwningComponent);

		if (OwningComponent && OwningComponent->GetMesh().IsValid())
		{
			const int32 BoneCount = OwningComponent->GetMesh()->GetData().RefSkeleton.BoneInfo.size();
			return BoneCount;
		}

		return 0;
	}

#if WITH_EDITOR
	AnimatorSkeletalMeshPreview::AnimatorSkeletalMeshPreview( class AssetPreviewSkeletalMeshGuiLayer* InPreview )
	{
		Preview = InPreview;
	}

	void AnimatorSkeletalMeshPreview::Tick( float DeltaTime )
	{
		const int32 BoneCount = Preview->BonePreviewTransforms.size();
		FinalBoneTranforms.resize(BoneCount);

		if (OwningComponent && OwningComponent->GetMesh().IsValid())
		{
			const ReferenceSkeleton& RefSkeleton = OwningComponent->GetMesh()->GetData().RefSkeleton;

			for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
			{
				const int32 ParentBoneIndex = RefSkeleton.BoneInfo[BoneIndex].ParentIndex;

				if (ParentBoneIndex == -1)
				{
					FinalBoneTranforms[BoneIndex] = Preview->BonePreviewTransforms[BoneIndex];
				}
				else
				{
					FinalBoneTranforms[BoneIndex] = Preview->BonePreviewTransforms[BoneIndex] * FinalBoneTranforms[ParentBoneIndex];
				}
			}

			for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
			{
				FinalBoneTranforms[BoneIndex] = RefSkeleton.BonePose[BoneIndex] * FinalBoneTranforms[BoneIndex];
			}
		}
	}

	const Matrix& AnimatorSkeletalMeshPreview::GetFinalBoneMatrix( int32 BoneIndex ) const
	{
		drn_check(BoneIndex >= 0);
		drn_check(BoneIndex < FinalBoneTranforms.size());

		return FinalBoneTranforms[BoneIndex];
	}

	int32 AnimatorSkeletalMeshPreview::GetBoneCount() const
	{
		return FinalBoneTranforms.size();
	}

// -----------------------------------------------------------------------------------

	AnimatorAnimationSequencePreview::AnimatorAnimationSequencePreview(AssetPreviewAnimationSequenceGuiLayer* InPreview )
		: AnimTime(0.0f)
	{
		Preview = InPreview;
	}

	void AnimatorAnimationSequencePreview::Tick( float DeltaTime )
	{
		OwningComponent->MarkRenderStateDirty();

		AnimationData& AnimData = Preview->m_OwningAsset->Data;
		const ReferenceSkeleton& RefSkeleton = Preview->m_OwningAsset->OwningSkeleton->GetData().RefSkeleton;

		AnimTime = std::fmod(AnimTime + DeltaTime * Preview->PreviewSpeed, AnimData.Length);

		int32 FrameIndex1; int32 FrameIndex2; float Alpha;
		AnimationRuntime::GetFrameIndicesFromTime(FrameIndex1, FrameIndex2, Alpha, AnimTime, Preview->m_OwningAsset->GetNumFrames(), AnimData.Length);

		if (Preview->DisplayFrameNumber >= 0)
		{
			FrameIndex1 = std::clamp<int32>(Preview->DisplayFrameNumber, 0, Preview->m_OwningAsset->GetNumFrames() - 1);
			Alpha = 0.0f;
		}

		if (Preview->StepAnimation)
		{
			Alpha = 0.0f;
		}

		//FrameIndex1 = 0;
		//FrameIndex2 = 1;
		//Alpha = 0.0f;

		AnimationKeyFrame& Frame1 = AnimData.KeyFrames[FrameIndex1];
		AnimationKeyFrame& Frame2 = AnimData.KeyFrames[FrameIndex2];

		const int32 BoneCount = RefSkeleton.BoneInfo.size();
		FinalBoneTranforms.resize(BoneCount);

		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			const int32 ParentBoneIndex = RefSkeleton.BoneInfo[BoneIndex].ParentIndex;

			Transform BoneTransform = Transform::Blend(Frame1.BonePose[BoneIndex], Frame2.BonePose[BoneIndex], Alpha);

			if (ParentBoneIndex == -1)
			{
				FinalBoneTranforms[BoneIndex] = BoneTransform;
			}
			else
			{
				FinalBoneTranforms[BoneIndex] = BoneTransform * FinalBoneTranforms[ParentBoneIndex];
			}
		}
		
		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			FinalBoneTranforms[BoneIndex] = RefSkeleton.BonePose[BoneIndex] * FinalBoneTranforms[BoneIndex];
		}
	}

	const Matrix& AnimatorAnimationSequencePreview::GetFinalBoneMatrix( int32 BoneIndex ) const
	{
		drn_check(BoneIndex >= 0);
		drn_check(BoneIndex < FinalBoneTranforms.size());

		return FinalBoneTranforms[BoneIndex];
	}

	int32 AnimatorAnimationSequencePreview::GetBoneCount() const
	{
		return FinalBoneTranforms.size();
	}

	Transform AnimatorAnimationSequencePreview::GetBoneWorldTransform(int32 BoneIndex) const
	{
		drn_check(BoneIndex >= 0);
		drn_check(BoneIndex < FinalBoneTranforms.size());

		const ReferenceSkeleton& RefSkeleton = Preview->m_OwningAsset->OwningSkeleton->GetData().RefSkeleton;
		return Matrix(RefSkeleton.BonePose[BoneIndex]).Inverse() * FinalBoneTranforms[BoneIndex];
	}

	float AnimatorAnimationSequencePreview::GetCurrentTime() const
	{
		AnimationData& AnimData = Preview->m_OwningAsset->Data;
		return Preview->DisplayFrameNumber >= 0 ? AnimData.Length * Preview->DisplayFrameNumber / AnimData.KeyFrames.size() : AnimTime;
	}

	int32 AnimatorAnimationSequencePreview::GetCurrentFrame() const
	{
		AnimationData& AnimData = Preview->m_OwningAsset->Data;
		return Preview->DisplayFrameNumber >= 0 ? Preview->DisplayFrameNumber : (AnimTime / AnimData.Length) * AnimData.KeyFrames.size();
	}

#endif
}  // namespace Drn