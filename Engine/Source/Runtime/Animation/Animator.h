#pragma once

#include "ForwardTypes.h"

#include "Runtime/Animation/AnimTask_PlayBlendSpace1D.h"
#include "Runtime/Animation/AnimationPose.h"

namespace Drn
{
	class Animator : public RefCountedObject
	{
	public:
		virtual void Tick(float DeltaTime);

		virtual Matrix GetFinalBoneMatrix(int32 BoneIndex) const = 0;
		virtual int32 GetBoneCount() const = 0;

		inline void SetOwningComponent( SkeletalMeshComponent* InOwningComponent ) { OwningComponent = InOwningComponent; }

	protected:
		SkeletalMeshComponent* OwningComponent;
	};

	class AnimatorReferencePose : public Animator
	{
	public:
		virtual void Tick(float DeltaTime) override;

		virtual Matrix GetFinalBoneMatrix(int32 BoneIndex) const override;
		virtual int32 GetBoneCount() const override;

	};

	class AnimatorAnimationSequence : public Animator
	{
	public:
		AnimatorAnimationSequence(AssetHandle<AnimationSequence> InAnimation);

		virtual void Tick(float DeltaTime) override;

		virtual Matrix GetFinalBoneMatrix(int32 BoneIndex) const override;
		virtual int32 GetBoneCount() const override;

		AssetHandle<AnimationSequence> Animation;
		std::vector<Transform> FinalBoneTranforms;

		float AnimTime;
	};

#if WITH_EDITOR
	class AnimatorSkeletalMeshPreview : public Animator
	{
	public:
		AnimatorSkeletalMeshPreview(class AssetPreviewSkeletalMeshGuiLayer* InPreview);

		virtual void Tick(float DeltaTime) override;

		virtual Matrix GetFinalBoneMatrix(int32 BoneIndex) const override;
		virtual int32 GetBoneCount() const override;

		class AssetPreviewSkeletalMeshGuiLayer* Preview;

		std::vector<Transform> FinalBoneTranforms;
	};

	class AnimatorAnimationSequencePreview : public Animator
	{
	public:
		AnimatorAnimationSequencePreview(class AssetPreviewAnimationSequenceGuiLayer* InPreview);

		virtual void Tick(float DeltaTime) override;

		virtual Matrix GetFinalBoneMatrix(int32 BoneIndex) const override;
		virtual int32 GetBoneCount() const override;

		Transform GetBoneWorldTransform(int32 BoneIndex) const;

		float GetCurrentTime() const;
		int32 GetCurrentFrame() const;

		class AssetPreviewAnimationSequenceGuiLayer* Preview;

		std::vector<Transform> FinalBoneTranforms;

		float AnimTime;
	};

	class AnimatorBlendSpace1DPreview : public Animator
	{
	public:
		AnimatorBlendSpace1DPreview(class AssetPreviewBlendSpace1DGuiLayer* InPreview);

		virtual void Tick(float DeltaTime) override;

		virtual Matrix GetFinalBoneMatrix(int32 BoneIndex) const override;
		virtual int32 GetBoneCount() const override;

		AnimTask_PlayBlendSpace1D PlayBlendSpace;

		AnimationPose FinalPose;

		class AssetPreviewBlendSpace1DGuiLayer* Preview;
	};
#endif
}