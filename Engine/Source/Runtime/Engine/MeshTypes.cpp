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
			uint8 T;
			Ar >> T;
			Type = (EMaterialType)T;

			if (Type == EMaterialType::Material)
			{
				std::string MaterialPath;
				Ar >> MaterialPath;
				MaterialHandle = AssetHandle<Material>( MaterialPath );
				MaterialHandle.LoadChecked();
			}
			else if (Type == EMaterialType::MaterialInstance)
			{
				std::string MaterialPath;
				Ar >> MaterialPath;
				MaterialInstanceHandle = AssetHandle<MaterialInstance>( MaterialPath );
				MaterialInstanceHandle.LoadChecked();
			}
			else
			{
				drn_check(false);
			}
		}

		else
		{
			Ar << (uint8)Type;
			if (Type == EMaterialType::Material)
			{
				Ar << MaterialHandle.GetPath();
			}
			else if (Type == EMaterialType::MaterialInstance)
			{
				Ar << MaterialInstanceHandle.GetPath();
			}
			else
			{
				drn_check(false);
			}
		}
	}

	Material* MaterialSlot::GetParentMaterial() const
	{
		if (GetMaterialInterface())
		{
			return GetMaterialInterface()->GetMaterial();
		}

		return nullptr;
	}

	MaterialInterface* MaterialSlot::GetMaterialInterface() const
	{
		switch ( Type )
		{
		case EMaterialType::Material:					return MaterialHandle.Get();
		case EMaterialType::MaterialInstance:			return MaterialInstanceHandle.Get();
		case EMaterialType::MaterialInstanceDynamic:	return MaterialInstanceDynamicHandle;
		default:										drn_check(false); return nullptr;
		}
	}

	std::string MaterialSlot::GetMaterialPath() const
	{
		switch ( Type )
		{
		case EMaterialType::Material:					return MaterialHandle.GetPath();
		case EMaterialType::MaterialInstance:			return MaterialInstanceHandle.GetPath();
		default:										drn_check(false); return NAME_NULL;
		}
	}

	void MaterialSlot::SetMaterial( AssetHandle<Material> InMaterial )
	{
		Type = EMaterialType::Material;
		MaterialHandle = InMaterial;
	}

	void MaterialSlot::SetMaterial( AssetHandle<MaterialInstance> InMaterial )
	{
		Type = EMaterialType::MaterialInstance;
		MaterialInstanceHandle = InMaterial;
	}

	void MaterialSlot::SetMaterial( TRefCountPtr<MaterialInstanceDynamic> InMaterial )
	{
		Type = EMaterialType::MaterialInstanceDynamic;
		MaterialInstanceDynamicHandle = InMaterial;
	}

	void MaterialSlot::LoadChecked()
	{
		switch ( Type )
		{
		case EMaterialType::Material:					return MaterialHandle.LoadChecked();
		case EMaterialType::MaterialInstance:			return MaterialInstanceHandle.LoadChecked();
		case EMaterialType::MaterialInstanceDynamic:	break;
		default:										drn_check(false);
		}
	}

	void MaterialSlot::Load()
	{
		switch ( Type )
		{
		case EMaterialType::Material:					return MaterialHandle.Load();
		case EMaterialType::MaterialInstance:			return MaterialInstanceHandle.Load();
		case EMaterialType::MaterialInstanceDynamic:	break;
		default:										drn_check(false);
		}
	}

	bool MaterialSlot::IsValid() const
	{
		switch ( Type )
		{
		case EMaterialType::Material:					return MaterialHandle.IsValid();
		case EMaterialType::MaterialInstance:			return MaterialInstanceHandle.IsValid();
		case EMaterialType::MaterialInstanceDynamic:	return MaterialInstanceDynamicHandle.IsValid();
		default:										drn_check(false); return true;
		}
	}

	std::string MaterialSlot::GetMaterialName() const
	{
		switch ( Type )
		{
		case EMaterialType::Material:					return Path::GetCleanName(MaterialHandle.GetPath());
		case EMaterialType::MaterialInstance:			return Path::GetCleanName(MaterialInstanceHandle.GetPath());
		//case EMaterialType::MaterialInstanceDynamic:	return Path::GetCleanName(MaterialInstanceDynamicHandle->get().GetPath());
		case EMaterialType::MaterialInstanceDynamic:	return "MID";
		default:										drn_check(false); return NAME_NULL;
		}
	}

	void MaterialProperty::Serialize( Archive& Ar )
	{
		MaterialSlot::Serialize(Ar);

		if ( Ar.IsLoading() )
		{
			Ar >> m_Name;
		}
		else
		{
			Ar << m_Name;
		}
	}

	void MaterialPropertyOverride::Serialize( Archive& Ar )
	{
		MaterialProperty::Serialize(Ar);

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
	void MaterialPropertyOverride::Draw( StaticMeshComponent* MC, uint32 MaterialIndex)
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
			const std::string MaterialPath = GetMaterialName();
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
						SetMaterial(AssetHandle<Material>(AssetPath));
						MC->MarkRenderStateDirty();
					}

					if (Type == EAssetType::MaterialInstance)
					{
						SetMaterial(AssetHandle<MaterialInstance>(AssetPath));
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

	void MaterialPropertyOverride::Draw( InstancedStaticMeshComponent* MC, uint32 MaterialIndex )
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
			const std::string MaterialPath = GetMaterialName();
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
						SetMaterial(AssetHandle<Material>(AssetPath));
						MC->MarkRenderStateDirty();
					}

					if (Type == EAssetType::MaterialInstance)
					{
						SetMaterial(AssetHandle<MaterialInstance>(AssetPath));
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