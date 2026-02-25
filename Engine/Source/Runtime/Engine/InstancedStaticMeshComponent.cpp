#include "DrnPCH.h"
#include "InstancedStaticMeshComponent.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#endif

namespace Drn
{
	InstancedStaticMeshComponent::InstancedStaticMeshComponent()
		: PrimitiveComponent()
		, m_StaticMeshSceneProxy(nullptr)
		, MinDrawDistance(0)
		, MaxDrawDistance(0)
	{}

	InstancedStaticMeshComponent::~InstancedStaticMeshComponent()
	{
		
	}

	void InstancedStaticMeshComponent::Tick( float DeltaTime )
	{
		PrimitiveComponent::Tick(DeltaTime);

	}

	void InstancedStaticMeshComponent::SetMesh( const AssetHandle<StaticMesh>& InHandle )
	{
		Mesh = InHandle;
		MarkRenderStateDirty();

		RefreshOverrideMaterials();
	}

	void InstancedStaticMeshComponent::Serialize( Archive& Ar )
	{
		PrimitiveComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			std::string Path;
			Ar >> Path;
			
			AssetHandle<StaticMesh> M = AssetHandle<StaticMesh>(Path);
			M.LoadChecked();
			
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

			Ar >> MinDrawDistance;
			Ar >> MaxDrawDistance;
		}
		
		else
		{
			Ar << Mesh.GetPath();

			Ar << uint16(m_OverrideMaterials.size());
			for (MaterialPropertyOverride& OD : m_OverrideMaterials)
			{
				OD.Serialize(Ar);
			}

			Ar << MinDrawDistance;
			Ar << MaxDrawDistance;
		}
	}

	void InstancedStaticMeshComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		//if (Mesh.IsValid())
		//{
		//	m_BodyInstance.InitBody(Mesh->GetBodySetup(), this, GetWorld()->GetPhysicScene());
		//}
		//
		//m_StaticMeshSceneProxy = new StaticMeshSceneProxy(this);
		//InOwningWorld->GetScene()->RegisterPrimitiveProxy(m_StaticMeshSceneProxy);
		//m_SceneProxy = m_StaticMeshSceneProxy;
	}

	void InstancedStaticMeshComponent::UnRegisterComponent()
	{
		//if (m_SceneProxy)
		//{
		//	m_SceneProxy->MarkPendingKill();
		//	m_SceneProxy = nullptr;
		//	m_StaticMeshSceneProxy= nullptr;
		//}
		//
		//if (Mesh.IsValid())
		//{
		//	m_BodyInstance.TermBody();
		//}

		PrimitiveComponent::UnRegisterComponent();
	}

	void InstancedStaticMeshComponent::SetMaterial( uint16 MaterialIndex, AssetHandle<Material>& InMaterial )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].SetMaterial(InMaterial);
			m_OverrideMaterials[MaterialIndex].m_Overriden = true;
			MarkRenderStateDirty();
		}
	}

	void InstancedStaticMeshComponent::SetMaterial( uint16 MaterialIndex, AssetHandle<MaterialInstance>& InMaterial )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].SetMaterial(InMaterial);
			m_OverrideMaterials[MaterialIndex].m_Overriden = true;
			MarkRenderStateDirty();
		}
	}

	void InstancedStaticMeshComponent::SetMaterial( uint16 MaterialIndex, TRefCountPtr<MaterialInstanceDynamic> InMaterial )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].SetMaterial(InMaterial);
			m_OverrideMaterials[MaterialIndex].m_Overriden = true;
			MarkRenderStateDirty();
		}
	}

	void InstancedStaticMeshComponent::RefreshOverrideMaterials()
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
					MaterialPropertyOverride MOD;
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

	void InstancedStaticMeshComponent::SetMinDrawDistance( float Value )
	{
		MinDrawDistance = Value;
		if (m_StaticMeshSceneProxy)
		{
			//m_StaticMeshSceneProxy->MinDrawDistance = MinDrawDistance;
		}
	}

	void InstancedStaticMeshComponent::SetMaxDrawDistance( float Value )
	{
		MaxDrawDistance = Value;
		if (m_StaticMeshSceneProxy)
		{
			//m_StaticMeshSceneProxy->MaxDrawDistance = MaxDrawDistance;
		}
	}

	BoxSphereBounds InstancedStaticMeshComponent::GetBounds()
	{
		BoxSphereBounds Bounds;
		if (Mesh.IsValid())
		{
			Bounds = Mesh->GetBounds();
			Bounds = Bounds.TransformBy(GetWorldTransform());
		}
		return Bounds;
	}

	BoxSphereBounds InstancedStaticMeshComponent::CalcBounds( const Transform& LocalToWorld ) const
	{
		if (Mesh.IsValid())
		{
			BoxSphereBounds Bounds;
			Bounds = Mesh->GetBounds();
			Bounds = Bounds.TransformBy(LocalToWorld);

			return Bounds;
		}

		return PrimitiveComponent::CalcBounds(LocalToWorld);
	}

#if WITH_EDITOR

	void InstancedStaticMeshComponent::DrawDetailPanel( float DeltaTime )
	{
		if (ImGui::Checkbox("Static", &bStatic))
		{
			SetStatic(bStatic);
		}

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

		if (ImGui::InputFloat("MinDrawDistance", &MinDrawDistance))
		{
			SetMinDrawDistance(MinDrawDistance);
		}

		if (ImGui::InputFloat("MaxDrawDistance", &MaxDrawDistance))
		{
			SetMaxDrawDistance(MaxDrawDistance);
		}
	}

	void InstancedStaticMeshComponent::ClearMesh()
	{
		Mesh = AssetHandle<StaticMesh>("");
	}

	void InstancedStaticMeshComponent::UpdateMeshWithPath( const char* NewPath )
	{
		AssetHandle<Asset> NewMesh(NewPath);
		EAssetType Type = NewMesh.LoadGeneric();

		if (NewMesh.IsValid() && Type == EAssetType::StaticMesh)
		{
			SetMesh(AssetHandle<StaticMesh>(NewPath));
		}
	}

	bool InstancedStaticMeshComponent::IsUsingMaterial( const AssetHandle<Material>& Mat )
	{
		for (const MaterialPropertyOverride& MD : m_OverrideMaterials)
		{
			if (MD.m_Overriden && MD.GetMaterialInterface() && MD.GetMaterialInterface()->IsDependent(*Mat))
			{
				return true;
			}
		}

		if (Mesh.IsValid())
		{
			for (const MaterialProperty& MD : Mesh->Data.Materials)
			{
				if (MD.GetMaterialInterface() && MD.GetMaterialInterface()->IsDependent(*Mat))
				{
					return true;
				}
			}
		}

		return false;
	}

	void InstancedStaticMeshComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		PrimitiveComponent::SetSelectedInEditor(SelectedInEditor);
	
		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectedInEditor( SelectedInEditor );
		}
	}

	void InstancedStaticMeshComponent::SetSelectable( bool Selectable )
	{
		PrimitiveComponent::SetSelectable(Selectable);

		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectable(Selectable);
		}
	}

	void InstancedStaticMeshComponent::DrawEditorDefault()
	{
		PrimitiveComponent::DrawEditorDefault();
	}

	void InstancedStaticMeshComponent::DrawEditorSelected()
	{
		PrimitiveComponent::DrawEditorSelected();

		BoxSphereBounds Bounds = GetBounds();
		GetWorld()->DrawDebugSphere(Bounds.Origin, Quat::Identity, Color::Green, Bounds.SphereRadius, 32, 0.0f, 0.0f);
		GetWorld()->DrawDebugBox(Box(Bounds.BoxExtent * -1, Bounds.BoxExtent), Transform(Bounds.Origin, Quat::Identity), Color::Blue, 0.0f, 0.0f);
	}
#endif

}  // namespace Drn