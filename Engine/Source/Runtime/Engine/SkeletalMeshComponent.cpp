#include "DrnPCH.h"
#include "SkeletalMeshComponent.h"
#include "Runtime/Renderer/SkeletalMeshSceneProxy.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#endif

namespace Drn
{
	SkeletalMeshComponent::SkeletalMeshComponent()
		: PrimitiveComponent()
		, m_SkeletalMeshSceneProxy(nullptr)
		, MinDrawDistance(0)
		, MaxDrawDistance(0)
	{
		
	}

	SkeletalMeshComponent::~SkeletalMeshComponent()
	{
		
	}

	void SkeletalMeshComponent::Tick( float DeltaTime )
	{
		PrimitiveComponent::Tick(DeltaTime);
	}

	void SkeletalMeshComponent::SetMesh( const AssetHandle<SkeletalMesh>& InHandle )
	{
		Mesh = InHandle;
		MarkRenderStateDirty();

		RefreshOverrideMaterials();
		UpdateBounds();

		//if (IsRegistered())
		//{
		//	RecreatePhysicState();
		//}
	}

	void SkeletalMeshComponent::Serialize( Archive& Ar )
	{
		PrimitiveComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			std::string Path;
			Ar >> Path;
			
			AssetHandle<SkeletalMesh> M = AssetHandle<SkeletalMesh>(Path);
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

	void SkeletalMeshComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		//CreatePhysicState();

		m_SkeletalMeshSceneProxy = new SkeletalMeshSceneProxy(this);
		InOwningWorld->GetScene()->RegisterPrimitiveProxy(m_SkeletalMeshSceneProxy);
		m_SceneProxy = m_SkeletalMeshSceneProxy;
	}

	void SkeletalMeshComponent::UnRegisterComponent()
	{
		//DestroyPhysicState();

		if (m_SceneProxy)
		{
			m_SceneProxy->MarkPendingKill();
			m_SceneProxy = nullptr;
			m_SkeletalMeshSceneProxy = nullptr;
		}

		PrimitiveComponent::UnRegisterComponent();
	}

	void SkeletalMeshComponent::OnUpdateTransform(bool SkipPhysic)
	{
		PrimitiveComponent::OnUpdateTransform(SkipPhysic);

		UpdateBounds();
	}

	//void SkeletalMeshComponent::CreatePhysicState()
	//{
	//	if (GetBodySetup() && GetBodySetup()->HasCollision())
	//	{
	//		m_BodyInstance.InitBody(Mesh->GetBodySetup(), GetWorldTransform(), this, GetWorld()->GetPhysicScene());
	//		bPhysicStateCreated = true;
	//	}
	//}
	//
	//void SkeletalMeshComponent::DestroyPhysicState()
	//{
	//	m_BodyInstance.TermBody();
	//	bPhysicStateCreated = false;
	//}
	//
	//void SkeletalMeshComponent::RecreatePhysicState()
	//{
	//	drn_check(IsRegistered());
	//
	//	DestroyPhysicState();
	//	CreatePhysicState();
	//}

	void SkeletalMeshComponent::SetMaterial( uint16 MaterialIndex, AssetHandle<Material>& InMaterial )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].SetMaterial(InMaterial);
			m_OverrideMaterials[MaterialIndex].m_Overriden = true;
			MarkRenderStateDirty();
		}
	}

	void SkeletalMeshComponent::SetMaterial( uint16 MaterialIndex, AssetHandle<MaterialInstance>& InMaterial )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].SetMaterial(InMaterial);
			m_OverrideMaterials[MaterialIndex].m_Overriden = true;
			MarkRenderStateDirty();
		}
	}

	void SkeletalMeshComponent::SetMaterial( uint16 MaterialIndex, TRefCountPtr<MaterialInstanceDynamic> InMaterial )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].SetMaterial(InMaterial);
			m_OverrideMaterials[MaterialIndex].m_Overriden = true;
			MarkRenderStateDirty();
		}
	}

	void SkeletalMeshComponent::SetMaterialOverride( uint16 MaterialIndex, bool bOverride )
	{
		if (MaterialIndex < m_OverrideMaterials.size())
		{
			m_OverrideMaterials[MaterialIndex].m_Overriden = bOverride;
			MarkRenderStateDirty();
		}
	}

#if WITH_EDITOR

	void SkeletalMeshComponent::DrawDetailPanel( float DeltaTime )
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

	void SkeletalMeshComponent::ClearMesh()
	{
		Mesh = AssetHandle<SkeletalMesh>("");
	}

	void SkeletalMeshComponent::UpdateMeshWithPath( const char* NewPath )
	{
		AssetHandle<Asset> NewMesh(NewPath);
		EAssetType Type = NewMesh.LoadGeneric();

		if (NewMesh.IsValid() && Type == EAssetType::SkeletalMesh)
		{
			AssetHandle<SkeletalMesh> MeshAsset(NewPath);
			MeshAsset.Load();

			SetMesh(MeshAsset);
		}
	}


	bool SkeletalMeshComponent::IsUsingMaterial( const AssetHandle<Material>& Mat )
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

	void SkeletalMeshComponent::SetSelectedInEditor(bool SelectedInEditor, const HitProxyData& Data)
	{
		PrimitiveComponent::SetSelectedInEditor(SelectedInEditor, Data);
	
		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectedInEditor( SelectedInEditor );
		}
	}

	void SkeletalMeshComponent::SetSelectable( bool Selectable )
	{
		PrimitiveComponent::SetSelectable(Selectable);

		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectable(Selectable);
		}
	}

	void SkeletalMeshComponent::DrawEditorDefault()
	{
		PrimitiveComponent::DrawEditorDefault();
		
	}

	void SkeletalMeshComponent::DrawEditorSelected()
	{
		PrimitiveComponent::DrawEditorSelected();

		if (GetWorld()->HasViewFlag(EWorldViewFlag::Bounds))
		{
			BoxSphereBounds Bounds = GetBounds();
			GetWorld()->DrawDebugSphere(Bounds.Origin, Quat::Identity, Color::Green, Bounds.SphereRadius, 32, 0.0f, 0.0f);
			GetWorld()->DrawDebugBox(Box(Bounds.BoxExtent * -1, Bounds.BoxExtent), Transform(Bounds.Origin, Quat::Identity), Color::Blue, 0.0f, 0.0f);
		}
	}

#endif

	void SkeletalMeshComponent::RefreshOverrideMaterials()
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

	void SkeletalMeshComponent::SetMinDrawDistance( float Value )
	{
		MinDrawDistance = Value;
		if (m_SkeletalMeshSceneProxy)
		{
			m_SkeletalMeshSceneProxy->MinDrawDistance = MinDrawDistance;
		}
	}

	void SkeletalMeshComponent::SetMaxDrawDistance( float Value )
	{
		MaxDrawDistance = Value;
		if (m_SkeletalMeshSceneProxy)
		{
			m_SkeletalMeshSceneProxy->MaxDrawDistance = MaxDrawDistance;
		}
	}

	BoxSphereBounds SkeletalMeshComponent::CalcBounds( const Transform& LocalToWorld ) const
	{
		if (Mesh.IsValid())
		{
			return Mesh->GetBounds().TransformBy(LocalToWorld);
		}

		return PrimitiveComponent::CalcBounds(LocalToWorld);
	}

        }  // namespace Drn