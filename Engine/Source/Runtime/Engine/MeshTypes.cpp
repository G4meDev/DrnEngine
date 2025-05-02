#include "DrnPCH.h"
#include "MeshTypes.h"

#if WITH_EDITOR
#include <imgui.h>
#include "Editor/EditorConfig.h"
#endif

namespace Drn
{
	void MaterialData::Serialize( Archive& Ar )
	{
	
		if ( Ar.IsLoading() )
		{
			Ar >> m_Name;
	
			std::string MaterialPath;
			Ar >> MaterialPath;
			m_Material = AssetHandle<Material>( MaterialPath );
			m_Material.Load();
		}
		else
		{
			Ar << m_Name;
			Ar << m_Material.GetPath();
		}
	}

	void MaterialOverrideData::Serialize( Archive& Ar )
	{
		MaterialData::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Overriden;
		}

		else
		{
			Ar << m_Overriden;
		}
	}

#if WITH_EDITOR
	void MaterialOverrideData::Draw( StaticMeshComponent* MC, uint32 MaterialIndex)
	{
		ImGui::PushID( "MaterialName" );
		ImGui::Text(m_Name.c_str());
		ImGui::PopID();

		if(ImGui::Checkbox( "##Checkbox" , &m_Overriden ))
			MC->MarkRenderStateDirty();

		ImGui::SameLine();

		if (m_Overriden)
		{
			const std::string MaterialPath = m_Material.GetPath() != "" ? m_Material.GetPath().c_str() : "..";
			ImGui::PushID( "MaterialPath" );
			ImGui::Text(MaterialPath.c_str());
			ImGui::PopID();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
				{
					auto AssetPath = static_cast<const char*>(payload->Data);
					AssetHandle<Asset> DropedMaterial(AssetPath);
					EAssetType Type = DropedMaterial.LoadGeneric();
					
					if (Type == EAssetType::Material)
					{
						m_Material = AssetHandle<Material>(AssetPath);
						MC->MarkRenderStateDirty();
					}
				}

				ImGui::EndDragDropTarget();
			}
		}

		else
		{
			std::string MatPath = "..";
			if (MC->GetMesh())
			{
				AssetHandle<Material> Mat = MC->GetMesh()->GetMaterialAtIndex(MaterialIndex);
				MatPath = Mat.GetPath();
			}

			ImGui::PushID("MaterialPath");
			ImGui::Text(MatPath.c_str());
			ImGui::PopID();
		}
	}
#endif

}