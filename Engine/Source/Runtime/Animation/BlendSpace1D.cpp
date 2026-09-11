#include "DrnPCH.h"
#include "BlendSpace1D.h"

#include "Editor/AssetPreview/AssetPreviewBlendSpace1DGuiLayer.h"

#if WITH_EDITOR
#include "imgui.h"
#include "Editor/EditorConfig.h"
#endif

namespace Drn
{
	BlendSpace1D::BlendSpace1D( const std::string& Path )
		: Asset(Path)
		, InterplationSpeed(1.0f)
		, Range(0.0f)
		, RangeMin(0.0f)
		, RangeMax(0.0f)
	{
		Load();
	}

#if WITH_EDITOR
	BlendSpace1D::BlendSpace1D( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, InterplationSpeed(1.0f)
		, Range(0.0f)
		, RangeMin(0.0f)
		, RangeMax(0.0f)
	{
		Save();
	}
#endif

	BlendSpace1D::~BlendSpace1D()
	{
		
	}

	void BlendSpace1D::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			std::string SkeletonPath;
			Ar >> SkeletonPath;

			OwningSkeleton = AssetHandle<SkeletalMesh>(SkeletonPath);
			OwningSkeleton.Load();

			Ar >> InterplationSpeed;
			Ar.operator>> <uint8>(SampleData);

			EvalProperties();
		}

		else
		{
			Ar << OwningSkeleton.GetPath();

			Ar << InterplationSpeed;
			Ar.operator<< <uint8>(SampleData);
		}
	}

	EAssetType BlendSpace1D::GetAssetType()
	{
		return EAssetType::BlendSpace1D;
	}

	void BlendSpace1D::EvalProperties()
	{
		SortSamples();
		CalculateRange();
	}

	void BlendSpace1D::SortSamples()
	{
		SortedSampleIndices.clear();

		for (uint8 SampleIndex = 0; SampleIndex < SampleData.size(); SampleIndex++)
		{
			if (SampleData[SampleIndex].Animation.IsValid())
			{
				SortedSampleIndices.push_back(SampleIndex);
			}
		}

		std::sort(SortedSampleIndices.begin(), SortedSampleIndices.end(), [&](uint8 A, uint8 B) { return SampleData[A].SampleTime < SampleData[B].SampleTime; });
	}

	void BlendSpace1D::CalculateRange()
	{
		const int32 NumValidSamples = SortedSampleIndices.size();

		RangeMin = NumValidSamples > 0 ? SampleData[SortedSampleIndices[0]].SampleTime : 0.0f;
		RangeMax = NumValidSamples > 0 ? SampleData[SortedSampleIndices[NumValidSamples-1]].SampleTime : 0.0f;

		Range = RangeMax - RangeMin;
	}

#if WITH_EDITOR
	void BlendSpace1D::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewBlendSpace1DGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void BlendSpace1D::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}

	bool BlendSpace1DSampleData::Draw(BlendSpace1D* OwningBlendSpace)
	{
		bool bDirty = false;

		std::string AssetPath	= Animation.GetPath();
		std::string AssetName	= Path::ConvertShortPath(AssetPath);
		AssetName				= Path::RemoveFileExtension(AssetName);
		AssetName				= AssetName == "" ? "None" : AssetName;

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, EditorConfig::AssetInputColor);
		ImGui::Text( "%s", AssetName.c_str() );
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
			{
				auto AssetPath = static_cast<const char*>(payload->Data);

				AssetHandle<Asset> NewAsset(AssetPath);
				EAssetType Type = NewAsset.LoadType();

				if (Type == EAssetType::AnimationSequence && AssetPath != Animation.GetPath())
				{
					AssetHandle<AnimationSequence> NewAnimation(AssetPath);
					NewAnimation.Load();

					if (NewAnimation->GetSkeleton().GetPath() == OwningBlendSpace->GetSkeleton().GetPath())
					{
						Animation = NewAnimation;
						bDirty = true;
					}
				}
			}

			ImGui::EndDragDropTarget();
		}

		bDirty |= ImGui::InputFloat("Play Rate", &PlayRate);
		bDirty |= ImGui::InputFloat("Sample Time", &SampleTime);

		return bDirty;
	}
#endif

// -----------------------------------------------------------------------------------------

	class Archive& operator<<(Archive& Ar, const BlendSpace1DSampleData& Value)
	{
		Ar << Value.Animation.GetPath();

		Ar << Value.PlayRate;
		Ar << Value.SampleTime;

		return Ar;
	}

	class Archive& operator>>(Archive& Ar, BlendSpace1DSampleData& Value)
	{
		std::string AnimatonPath;
		Ar >> AnimatonPath;

		Value.Animation = AssetHandle<AnimationSequence>(AnimatonPath);
		Value.Animation.Load();

		Ar >> Value.PlayRate;
		Ar >> Value.SampleTime;

		return Ar;
	}

}