#include "DrnPCH.h"
#include "Animator.h"

#include "Editor/AssetPreview/AssetPreviewSkeletalMeshGuiLayer.h"

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
#endif

}  // namespace Drn