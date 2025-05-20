#include "DrnPCH.h"
#include "StaticMeshComponent.h"
#include "Runtime/Renderer/StaticMeshSceneProxy.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#endif

namespace Drn
{
	StaticMeshComponent::StaticMeshComponent()
		: PrimitiveComponent()
		, m_SceneProxy(nullptr)
	{
		
	}

	StaticMeshComponent::~StaticMeshComponent()
	{
		
	}

	void StaticMeshComponent::Tick( float DeltaTime )
	{
		PrimitiveComponent::Tick(DeltaTime);
	}

	void StaticMeshComponent::SetMesh( const AssetHandle<StaticMesh>& InHandle )
	{
		Mesh = InHandle;
		MarkRenderStateDirty();

		RefreshOverrideMaterials();
	}

	void StaticMeshComponent::Serialize( Archive& Ar )
	{
		PrimitiveComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			std::string Path;
			Ar >> Path;
			
			AssetHandle<StaticMesh> M = AssetHandle<StaticMesh>(Path);
			M.Load();
			
			SetMesh(M);

// --------------------------------------------------------------------

			uint16 size;
			Ar >> size;
			
			m_OverrideMaterials.clear();
			m_OverrideMaterials.resize(size);
			
			for (uint16 i = 0; i < size; i++)
			{
				m_OverrideMaterials[i].Serialize(Ar);
			}
			
			RefreshOverrideMaterials();
		}
		
		else
		{
			Ar << Mesh.GetPath();

			Ar << uint16(m_OverrideMaterials.size());
			for (MaterialOverrideData OD : m_OverrideMaterials)
			{
				OD.Serialize(Ar);
			}
		}
	}

	void StaticMeshComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		if (Mesh.IsValid())
		{
			m_BodyInstance.InitBody(Mesh->GetBodySetup(), this, GetWorld()->GetPhysicScene());
		}

		m_SceneProxy = new StaticMeshSceneProxy(this);
		InOwningWorld->GetScene()->RegisterPrimitiveProxy(m_SceneProxy);

	}

	void StaticMeshComponent::UnRegisterComponent()
	{
		m_SceneProxy->MarkPendingKill();
		if (GetWorld()->GetScene())
		{
			GetWorld()->GetScene()->UnRegisterPrimitiveProxy(m_SceneProxy);
		}

		//if (GetWorld()->GetScene())
		//{
		//	GetWorld()->GetScene()->RemovePrimitiveProxy(m_SceneProxy);
		//}
		//delete m_SceneProxy;

		if (Mesh.IsValid())
		{
			m_BodyInstance.TermBody();
		}

		PrimitiveComponent::UnRegisterComponent();
	}

	void StaticMeshComponent::SetMaterial( uint16 MaterialIndex, AssetHandle<Material>& InMaterial )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].m_Material = InMaterial;
			m_OverrideMaterials[MaterialIndex].m_Overriden = true;
			MarkRenderStateDirty();
		}
	}

#if WITH_EDITOR

	void StaticMeshComponent::DrawDetailPanel( float DeltaTime )
	{
		PrimitiveComponent::DrawDetailPanel(DeltaTime);

		ImGui::TextWrapped( "Guid: %s", m_Guid.ToString().c_str());

		if ( ImGui::Button( "Clear" ) )
		{
			ClearMesh();
		}

		std::string AssetPath	= Mesh.GetPath();
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
				UpdateMeshWithPath(AssetPath);
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::Separator();
		ImGui::TextWrapped(Mesh.GetPath().c_str());
		ImGui::Separator();

		if ( ImGui::CollapsingHeader( "Materials", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen) )
		{
			for (int i = 0; i < m_OverrideMaterials.size(); i++)
			{
				ImGui::PushID(i);
				m_OverrideMaterials[i].Draw(this, i);
				ImGui::PopID();
			}

			if ( ImGui::Button( "Refresh##Materials" ) )
			{
				RefreshOverrideMaterials();
			}
		}


	}

	void StaticMeshComponent::ClearMesh()
	{
		Mesh = AssetHandle<StaticMesh>("");
	}

	void StaticMeshComponent::UpdateMeshWithPath( const char* NewPath )
	{
		AssetHandle<Asset> NewMesh(NewPath);
		EAssetType Type = NewMesh.LoadGeneric();

		if (NewMesh.IsValid() && Type == EAssetType::StaticMesh)
		{
			SetMesh(AssetHandle<StaticMesh>(NewPath));
		}
	}


	bool StaticMeshComponent::IsUsingMaterial( const AssetHandle<Material>& Mat )
	{
		for (const MaterialOverrideData& MD : m_OverrideMaterials)
		{
			if (MD.m_Overriden && MD.m_Material.GetPath() == Mat.GetPath())
			{
				return true;
			}
		}

		if (Mesh.IsValid())
		{
			for (const MaterialData& MD : Mesh->Data.Materials)
			{
				if (MD.m_Material.GetPath() == Mat.GetPath())
				{
					return true;
				}
			}
		}

		return false;
	}

	void StaticMeshComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		PrimitiveComponent::SetSelectedInEditor(SelectedInEditor);
	
		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectedInEditor( SelectedInEditor );
		}
	}

	void StaticMeshComponent::SetSelectable( bool Selectable )
	{
		PrimitiveComponent::SetSelectable(Selectable);

		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectable(Selectable);
		}
	}

#endif

	void StaticMeshComponent::RefreshOverrideMaterials()
	{
		MarkRenderStateDirty();

		if (Mesh.IsValid())
		{
			for (int i = 0; i < Mesh->Data.Materials.size(); i++)
			{
				std::string& MaterialName = Mesh->Data.Materials[i].m_Name;

				if ( i < m_OverrideMaterials.size() )
				{
					m_OverrideMaterials[i].m_Name = MaterialName;
				}
				else
				{
					MaterialOverrideData MOD;
					MOD.m_Name = MaterialName;
					m_OverrideMaterials.push_back(MOD);
				}
			}

			for (int i = Mesh->Data.Materials.size(); i < m_OverrideMaterials.size(); i++)
			{
				m_OverrideMaterials.pop_back();
			}
		}

		else
		{
			m_OverrideMaterials.clear();
		}
	}

}