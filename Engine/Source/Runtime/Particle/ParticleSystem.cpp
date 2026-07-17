#include "DrnPCH.h"
#include "ParticleSystem.h"

#include "Runtime/Particle/ParticleEmitter.h"
#include "Editor/AssetPreview/AssetPreviewParticleSystemGuiLayer.h"

namespace Drn
{
	ParticleSystem::ParticleSystem( const std::string& Path )
		: Asset( Path )
		, bAutoDeactivate(true)
		, bUseFixedBounds(false)
		, FixedBoundsMin(-1.0f)
		, FixedBoundsMax(1.0f)
		, WarmupTime(0.0f)
		, WarmupTickRate(0.0f)
		, ThumbnailWarmup(1.0f)
		, ThumbnailDistance(2.0f)
	{
		Load();
	}

#if WITH_EDITOR
	ParticleSystem::ParticleSystem( const std::string& InPath, const std::string& InSourcePath )
		: Asset( InPath )
		, bAutoDeactivate(true)
		, bUseFixedBounds(false)
		, FixedBoundsMin(-1.0f)
		, FixedBoundsMax(1.0f)
		, WarmupTime(0.0f)
		, WarmupTickRate(0.0f)
		, ThumbnailWarmup(1.0f)
		, ThumbnailDistance(2.0f)
	{
		Save();
	}
#endif

	ParticleSystem::~ParticleSystem()
	{
		
	}

	void ParticleSystem::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bAutoDeactivate;
			Ar >> bUseFixedBounds;
			Ar >> FixedBoundsMin;
			Ar >> FixedBoundsMax;

			Ar >> WarmupTime;
			Ar >> WarmupTickRate;
			Ar >> ThumbnailWarmup;
			Ar >> ThumbnailDistance;

			uint8 EmitterCount = 0;
			Ar >> EmitterCount;
			Emitters.resize(EmitterCount);
			for (int32 EmitterIndex = 0; EmitterIndex < EmitterCount; EmitterIndex++)
			{
				Emitters[EmitterIndex] = new ParticleEmitter();
				Emitters[EmitterIndex]->Serialize(Ar);
			}
		}

		else
		{
			Ar << bAutoDeactivate;
			Ar << bUseFixedBounds;
			Ar << FixedBoundsMin;
			Ar << FixedBoundsMax;

			Ar << WarmupTime;
			Ar << WarmupTickRate;
			Ar << ThumbnailWarmup;
			Ar << ThumbnailDistance;

			const uint8 EmitterCount = std::min(Emitters.size(), (size_t)UINT8_MAX);
			Ar << EmitterCount;
			for (int32 EmitterIndex = 0; EmitterIndex < EmitterCount; EmitterIndex++)
			{
				Emitters[EmitterIndex]->Serialize(Ar);
			}
		}
	}

	EAssetType ParticleSystem::GetAssetType()
	{
		return EAssetType::ParticleSystem;
	}

#if WITH_EDITOR

	void ParticleSystem::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewParticleSystemGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void ParticleSystem::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}

	bool ParticleSystem::Draw()
	{
		bool bDirty = false;

		if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= ImGui::Checkbox("Auto Deactivate", &bAutoDeactivate);
		}

		if (ImGui::CollapsingHeader("Bounds", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= ImGui::Checkbox("Use Fixed Bounds", &bUseFixedBounds);
			bDirty |= FixedBoundsMin.Draw("Fixed Bounds Min", "Fixed Bounds Min", EParameterPopupContext::None);
			bDirty |= FixedBoundsMax.Draw("Fixed Bounds Max", "Fixed Bounds Max", EParameterPopupContext::None);
		}

		if (ImGui::CollapsingHeader("Warmup", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::InputFloat("Warmup Time", &WarmupTime))
			{
				WarmupTime = std::max(WarmupTime, 0.0f);
				bDirty = true;
			}

			if (ImGui::InputFloat("Warmup Tick Rate", &WarmupTickRate))
			{
				WarmupTickRate = std::min(WarmupTickRate, WarmupTime);
				bDirty = true;
			}
		}

		if (ImGui::CollapsingHeader("Thumbnail", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= ImGui::InputFloat("Thumbnail Warmup", &ThumbnailWarmup);
			bDirty |= ImGui::InputFloat("Thumbnail Distance", &ThumbnailDistance);
		}

		return bDirty;
	}

#endif

}