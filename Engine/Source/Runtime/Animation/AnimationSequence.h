#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct AnimationKeyFrame
	{
		std::vector<Transform> BonePose;
		
		friend class Archive& operator<<(Archive& Ar, const AnimationKeyFrame& Value);
		friend class Archive& operator>>(Archive& Ar, AnimationKeyFrame& Value);
	};

	struct AnimationData
	{
		AnimationData()
			: Length(0.0f)
		{}

		float Length;
		std::vector<AnimationKeyFrame> KeyFrames;

		friend class Archive& operator<<(Archive& Ar, const AnimationData& Value);
		friend class Archive& operator>>(Archive& Ar, AnimationData& Value);
	};

	class AnimationSequence : public Asset
	{	
	public:
		AnimationSequence(const std::string& Path);
		virtual ~AnimationSequence();

#if WITH_EDITOR
		AnimationSequence(const std::string& InPath, const std::string& InSourcePath, AssetImportUserData& UserData);
#endif

		virtual void Serialize( Archive& Ar ) override;

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::AnimationSequence; }

		inline AssetHandle<SkeletalMesh> GetSkeleton() const { return OwningSkeleton; }
		inline const AnimationData& GetAnimationData() const { return Data; }

#if WITH_EDITOR
		void Import();

		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		class AssetPreviewAnimationSequenceGuiLayer* GuiLayer = nullptr;
#endif

		inline uint32 GetNumFrames() const { return Data.KeyFrames.size(); }

	private:

		std::string m_SourcePath;
		AssetHandle<SkeletalMesh> OwningSkeleton;

		AnimationData Data;

		friend class AssetPreviewAnimationSequenceGuiLayer;
		friend class AssetImporterSkeletalMesh;
		friend class AnimatorAnimationSequencePreview;
	};

	class AnimationRuntime
	{
	public:
		static void GetFrameIndicesFromTime(int32& OutKeyIndex1, int32& OutKeyIndex2, float& OutAlpha, const float Time, const int32 NumFrames, const float SequenceLength);
		static float StepAnimationTime(float CurrentTime, float DeltaTime, float AnimationLength, float PlayRate = 1.0f, bool bLoop = true);
	};
}