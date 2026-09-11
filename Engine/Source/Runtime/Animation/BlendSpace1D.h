#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct BlendSpace1DSampleData
	{
		BlendSpace1DSampleData()
			: PlayRate(1.0f)
			, SampleTime(0.0f)
		{}

		AssetHandle<AnimationSequence> Animation;
		float PlayRate;
		float SampleTime;

		friend class Archive& operator<<( Archive& Ar, const BlendSpace1DSampleData& Value );
		friend class Archive& operator>>( Archive& Ar, BlendSpace1DSampleData& Value );

#if WITH_EDITOR
		bool Draw(BlendSpace1D* OwningBlendSpace);
#endif
	};

	class BlendSpace1D : public Asset
	{	
	public:
		BlendSpace1D(const std::string& Path);
		virtual ~BlendSpace1D();

#if WITH_EDITOR
		BlendSpace1D(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual void Serialize( Archive& Ar ) override;

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::BlendSpace1D; }

		inline AssetHandle<SkeletalMesh> GetSkeleton() const { return OwningSkeleton; }

		inline const std::vector<uint8>& GetSortedSampleIndices() const { return SortedSampleIndices; };
		inline const std::vector<BlendSpace1DSampleData>& GetSampleData() const { return SampleData; }
		inline float GetInterplationSpeed() const { return InterplationSpeed; }
		inline float GetRange() const { return Range; }
		inline float GetRangeMin() const { return RangeMin; }
		inline float GetRangeMax() const { return RangeMax; }

		void EvalProperties();
		void SortSamples();
		void CalculateRange();

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		class AssetPreviewBlendSpace1DGuiLayer* GuiLayer = nullptr;
#endif

	private:

		AssetHandle<SkeletalMesh> OwningSkeleton;

		std::vector<BlendSpace1DSampleData> SampleData;
		float InterplationSpeed;

		std::vector<uint8> SortedSampleIndices;
		float Range;
		float RangeMin;
		float RangeMax;

		friend class AssetPreviewBlendSpace1DGuiLayer;
		friend class AssetImporterSkeletalMesh;
		friend class AnimatorAnimationSequencePreview;
	};
}