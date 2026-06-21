#include "DrnPCH.h"
#include "AssetPreviewPhysicalMaterialGuiLayer.h"

#if WITH_EDITOR

#include "imgui.h"

namespace Drn
{
	AssetPreviewPhysicalMaterialGuiLayer::AssetPreviewPhysicalMaterialGuiLayer( PhysicalMaterial* InOwningAsset )
	{
		m_OwningAsset = AssetHandle<PhysicalMaterial>( InOwningAsset->m_Path );
		m_OwningAsset.Load();
	}

	AssetPreviewPhysicalMaterialGuiLayer::~AssetPreviewPhysicalMaterialGuiLayer() 
	{
		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewPhysicalMaterialGuiLayer::Draw( float DeltaTime )
	{
		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{

			ImGui::End();
			return;
		}

		if (ImGui::Button("Save"))
		{
			m_OwningAsset->Save();
		}

		const char* FrictionModeOptions[] = { "Average", "Min", "Multiply", "Max" };

		ImGui::InputFloat("Friction", &m_OwningAsset->Friction);
		ImGui::InputFloat("Static Friction", &m_OwningAsset->StaticFriction);
		int32 CurrentFrictionBlend = (uint8)m_OwningAsset->FrictionCombineMode;
		ImGui::Combo("Friction Blend Mode", &CurrentFrictionBlend, FrictionModeOptions, IM_ARRAYSIZE(FrictionModeOptions));
		m_OwningAsset->FrictionCombineMode = (EFrictionCombineMode)CurrentFrictionBlend;
		ImGui::Checkbox("Override Friction Blend Mode", &m_OwningAsset->bOverrideFrictionCombineMode);

		ImGui::InputFloat("Restitution", &m_OwningAsset->Restitution);
		int32 CurrentRestitutionBlend = (uint8)m_OwningAsset->RestitutionCombineMode;
		ImGui::Combo("Restitution Blend Mode", &CurrentRestitutionBlend, FrictionModeOptions, IM_ARRAYSIZE(FrictionModeOptions));
		m_OwningAsset->RestitutionCombineMode = (EFrictionCombineMode)CurrentRestitutionBlend;
		ImGui::Checkbox("Override Restitution Blend Mode", &m_OwningAsset->bOverrideRestitutionCombineMode);

		int32 CurrentSurfaceType = (uint8)m_OwningAsset->SurfaceType;
		std::string& DisplayName = EngineTypes::Get()->SurfaceTypesDisplayNames[CurrentSurfaceType];
		if (ImGui::BeginCombo("Surface Type", DisplayName.c_str()))
		{
			for (int32 i = 0; i < 64; i++)
			{
				if (EngineTypes::Get()->SurfaceTypesDisplayNames[i] == "")
				{
					continue;
				}

				const bool bSelected = i == CurrentSurfaceType;
				if (ImGui::Selectable(EngineTypes::Get()->SurfaceTypesDisplayNames[i].c_str(), bSelected))
				{
					CurrentSurfaceType = i;
				}

				if (bSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
		m_OwningAsset->SurfaceType = (EPhysicalSurface)CurrentSurfaceType;

		ImGui::End();
	}
}

#endif