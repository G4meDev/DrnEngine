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

		return bDirty;
	}

#endif

}