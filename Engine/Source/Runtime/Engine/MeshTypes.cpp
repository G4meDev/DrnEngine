#include "DrnPCH.h"
#include "MeshTypes.h"

#if WITH_EDITOR
#include <imgui.h>
#include "Editor/EditorConfig.h"
#endif

namespace Drn
{
	void MaterialSlot::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			std::string MaterialPath;
			Ar >> MaterialPath;
			MaterialHandle = AssetHandle<Material>( MaterialPath );
			MaterialHandle.LoadChecked();
		}

		else
		{
			Ar << MaterialHandle.GetPath();
		}
	}

	Material* MaterialSlot::GetMaterial() const
	{
		return *MaterialHandle;
	}

	MaterialInterface* MaterialSlot::GetMaterialInterface() const
	{
		return *MaterialHandle;
	}

	std::string MaterialSlot::GetMaterialPath() const
	{
		return MaterialHandle.GetPath();
	}

	void MaterialSlot::LoadChecked()
	{
		MaterialHandle.LoadChecked();
	}

	void MaterialSlot::Load()
	{
		MaterialHandle.Load();
	}

	bool MaterialSlot::IsValid() const
	{
		return MaterialHandle.IsValid();
	}

	std::string MaterialSlot::GetMaterialName() const
	{
		return Path::GetCleanName(MaterialHandle.GetPath());
	}

	void MaterialData::Serialize( Archive& Ar )
	{
		if ( Ar.IsLoading() )
		{
			Ar >> m_Name;
			m_MaterialSlot.Serialize(Ar);
		}
		else
		{
			Ar << m_Name;
			m_MaterialSlot.Serialize(Ar);
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
			//const std::string MaterialPath = m_MaterialSlot.GetMaterialName() GetMaterial()->path() != "" ? m_Material.GetPath().c_str() : "..";
			const std::string MaterialPath = m_MaterialSlot.GetMaterialName();
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
						m_MaterialSlot = AssetHandle<Material>(AssetPath);
						MC->MarkRenderStateDirty();
					}
				}

				ImGui::EndDragDropTarget();
			}
		}

		else
		{
			std::string MatPath = "..";
			if (MC->GetMesh().IsValid())
			{
				MaterialSlot Mat = MC->GetMesh()->GetMaterialAtIndex(MaterialIndex);
				MatPath = Mat.GetMaterialName();
			}

			ImGui::PushID("MaterialPath");
			ImGui::Text(MatPath.c_str());
			ImGui::PopID();
		}
	}
#endif

        }