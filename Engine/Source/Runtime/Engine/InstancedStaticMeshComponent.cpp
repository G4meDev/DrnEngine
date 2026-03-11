#include "DrnPCH.h"
#include "InstancedStaticMeshComponent.h"
#include "Runtime/Engine/InstancedStaticMeshSceneProxy.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#endif

namespace Drn
{
	InstancedStaticMeshComponent::InstancedStaticMeshComponent()
		: PrimitiveComponent()
		, m_InstancedStaticMeshSceneProxy(nullptr)
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
		UpdateBounds();
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

// --------------------------------------------------------------------

			int32 InstanceCount;
			Ar >> InstanceCount;
			PerInstanceTransform.resize(InstanceCount);
			for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
			{
				Ar >> PerInstanceTransform[InstanceIndex];
			}

			for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
			{
				Ar >> bCustomData[CustomDataIndex];
				if (bCustomData[CustomDataIndex])
				{
					CustomData[CustomDataIndex].resize(InstanceCount);

					for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
					{
						Ar >> CustomData[CustomDataIndex][InstanceIndex];
					}
				}
			}

			UpdateBounds();

			Ar >> InstancingRandomSeed;
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

// --------------------------------------------------------------------

			const int32 InstanceCount = GetInstanceCount();
			Ar << InstanceCount;
			for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
			{
				Ar << PerInstanceTransform[InstanceIndex];
			}
			for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
			{
				Ar << bCustomData[CustomDataIndex];
				if (bCustomData[CustomDataIndex])
				{
					for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
					{
						Ar << CustomData[CustomDataIndex][InstanceIndex];
					}
				}
			}

			Ar << InstancingRandomSeed;
		}
	}

	void InstancedStaticMeshComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		//if (Mesh.IsValid())
		//{
		//	m_BodyInstance.InitBody(Mesh->GetBodySetup(), this, GetWorld()->GetPhysicScene());
		//}

		m_InstancedStaticMeshSceneProxy = new InstancedStaticMeshSceneProxy(this);
		InOwningWorld->GetScene()->RegisterPrimitiveProxy(m_InstancedStaticMeshSceneProxy);
		m_SceneProxy = m_InstancedStaticMeshSceneProxy;
	}

	void InstancedStaticMeshComponent::UnRegisterComponent()
	{
		if (m_SceneProxy)
		{
			m_SceneProxy->MarkPendingKill();
			m_SceneProxy = nullptr;
			m_InstancedStaticMeshSceneProxy = nullptr;
		}

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
		if (m_InstancedStaticMeshSceneProxy)
		{
			m_InstancedStaticMeshSceneProxy->MinDrawDistance = MinDrawDistance;
		}
	}

	void InstancedStaticMeshComponent::SetMaxDrawDistance( float Value )
	{
		MaxDrawDistance = Value;
		if (m_InstancedStaticMeshSceneProxy)
		{
			m_InstancedStaticMeshSceneProxy->MaxDrawDistance = MaxDrawDistance;
		}
	}

	BoxSphereBounds InstancedStaticMeshComponent::CalcBounds( const Transform& LocalToWorld ) const
	{
		if (Mesh.IsValid() && GetInstanceCount() > 0)
		{
			Matrix BoundTransformMatrix = LocalToWorld;

			BoxSphereBounds RenderBounds = Mesh->GetBounds();
			BoxSphereBounds Bounds = RenderBounds.TransformBy(PerInstanceTransform[0] * LocalToWorld);

			for (int32 InstanceIndex = 1; InstanceIndex < GetInstanceCount(); InstanceIndex++)
			{
				Bounds = Bounds + RenderBounds.TransformBy(PerInstanceTransform[InstanceIndex] * LocalToWorld);
			}

			return Bounds;
		}

		return BoxSphereBounds(LocalToWorld.GetLocation(), Vector::ZeroVector, 0.0f);
	}

	int32 InstancedStaticMeshComponent::AddInstance( const Transform& InstanceTransform )
	{
		PerInstanceTransform.push_back(InstanceTransform);
		for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
		{
			if (bCustomData[CustomDataIndex])
			{
				CustomData[CustomDataIndex].push_back(Vector4(0));
			}
		}

		UpdateBounds();
		MarkRenderStateDirty();
		return GetInstanceCount() - 1;
	}

	std::vector<int32> InstancedStaticMeshComponent::AddInstances( const std::vector<Transform>& InstanceTransforms, bool bShouldReturnIndices )
	{
		const int32 Count = InstanceTransforms.size();
		const int32 NewSize = GetInstanceCount() + Count;
		std::vector<int32> NewIndices;

		if (Count > 0)
		{
			if (bShouldReturnIndices)
			{
				NewIndices.reserve(Count);
			}

			int32 InstanceIndex = GetInstanceCount();
			PerInstanceTransform.reserve(NewSize);

			for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
			{
				if (bCustomData[CustomDataIndex])
				{
					CustomData[CustomDataIndex].reserve(NewSize);
				}
			}

			for (int32 i = 0; i < Count; i++)
			{
				PerInstanceTransform.push_back(InstanceTransforms[i]);

				for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
				{
					if (bCustomData[CustomDataIndex])
					{
						CustomData[CustomDataIndex].push_back(Vector4(0));
					}
				}

				if (bShouldReturnIndices)
				{
					NewIndices.push_back(InstanceIndex);
				}

				InstanceIndex++;
			}

			UpdateBounds();
			MarkRenderStateDirty();
		}

		return NewIndices;
	}

	int32 InstancedStaticMeshComponent::AddInstanceWorldSpace( const Transform& WorldTransform )
	{
		AddInstance(WorldTransform.GetRelativeTransform(GetWorldTransform()));

		return GetInstanceCount() - 1;
	}

	bool InstancedStaticMeshComponent::UpdateInstanceTransform( int32 InstanceIndex, const Transform& NewInstanceTransform, bool bWorldSapce, bool bMarkRenderStateDirty, bool bTeleport )
	{
		if (InstanceIndex < GetInstanceCount())
		{
			Matrix& InstanceTransform = PerInstanceTransform[InstanceIndex];
			InstanceTransform = bWorldSapce ? NewInstanceTransform.GetRelativeTransform(GetWorldTransform()) : NewInstanceTransform;

			if (bMarkRenderStateDirty)
			{
				UpdateBounds();
				MarkRenderStateDirty();
			}

			return true;
		}

		return false;
	}

	void InstancedStaticMeshComponent::SetCustomData( int32 DataIndex, int32 InstanceIndex, const Vector4& Value, bool bMarkRenderStateDirty )
	{
		drn_check(DataIndex < NUM_INSTANCED_CUSTOM_DATA && bCustomData[DataIndex]);
		drn_check(InstanceIndex < GetInstanceCount() && InstanceIndex < CustomData[DataIndex].size());

		CustomData[DataIndex][InstanceIndex] = Value;

		if (bMarkRenderStateDirty)
		{
			MarkRenderStateDirty();
		}
	}

	void InstancedStaticMeshComponent::SetCustomDataEnabled( int32 Index, bool bEnabled )
	{
		drn_check(Index < NUM_INSTANCED_CUSTOM_DATA);

		if (bEnabled && !bCustomData[Index])
		{
			CustomData[Index].resize(GetInstanceCount());
			MarkRenderStateDirty();
		}
		else if (!bEnabled && bCustomData[Index])
		{
			CustomData[Index].clear();
			MarkRenderStateDirty();
		}

		bCustomData[Index] = bEnabled;
	}

	bool InstancedStaticMeshComponent::GetInstanceTransform( int32 InstanceIndex, Transform& OutInstanceTransform, bool bWorldSpace ) const
	{
		if (InstanceIndex < GetInstanceCount())
		{
			OutInstanceTransform = PerInstanceTransform[InstanceIndex];
			if (bWorldSpace)
			{
				OutInstanceTransform = Transform(OutInstanceTransform) * GetWorldTransform();
			}

			return true;
		}

		return false;
	}

	bool InstancedStaticMeshComponent::RemoveInstance( int32 InstanceIndex )
	{
		if (InstanceIndex < GetInstanceCount())
		{
			PerInstanceTransform.erase(PerInstanceTransform.begin() + InstanceIndex);

			for (int32 i = 0; i < NUM_INSTANCED_CUSTOM_DATA; i++)
			{
				if (bCustomData[i])
				{
					drn_check(InstanceIndex < CustomData[i].size());
					CustomData[i].erase(CustomData[i].begin() + InstanceIndex);
				}
			}

			UpdateBounds();
			MarkRenderStateDirty();

			return true;
		}

		return false;
	}

	void InstancedStaticMeshComponent::ClearInstances()
	{
		PerInstanceTransform.clear();
		for (int32 i = 0; i < NUM_INSTANCED_CUSTOM_DATA; i++)
		{
			CustomData[i].clear();
		}

		UpdateBounds();
		MarkRenderStateDirty();
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

		for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
		{
			const std::string Label = "Custom Data " + std::to_string(CustomDataIndex);
			bool bEnabled = bCustomData[CustomDataIndex];

			if (ImGui::Checkbox(Label.c_str(), &bEnabled))
			{
				SetCustomDataEnabled(CustomDataIndex, bEnabled);
			}
		}

		DrawInstances();
	}

	void InstancedStaticMeshComponent::DrawInstances()
	{
		if (ImGui::CollapsingHeader("Instances"))
		{
			if (ImGui::Button("Add"))
			{
				AddInstance(Transform::Identity);
			}
			
			if (ImGui::Button("Remove"))
			{
				RemoveInstance(GetInstanceCount() - 1);
			}

			if (ImGui::Button("Clear"))
			{
				ClearInstances();
			}

			ImGui::Text( std::format("{} instances", GetInstanceCount()).c_str() );
			ImGui::Separator();

			for (int32 InstanceIndex = 0; InstanceIndex < GetInstanceCount(); InstanceIndex++)
			{
				std::string InstanceLabel = std::to_string(InstanceIndex).c_str();
				ImGui::Text(InstanceLabel.c_str());

				Transform InstanceTransform = PerInstanceTransform[InstanceIndex];
				if (InstanceTransform.Draw(InstanceLabel.c_str()))
				{
					UpdateInstanceTransform(InstanceIndex, InstanceTransform, false, true, true);
				}
				
				for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
				{
					if (bCustomData[CustomDataIndex])
					{
						Vector4 Data = CustomData[CustomDataIndex][InstanceIndex];
						if (Data.Draw(InstanceLabel.c_str(), std::format("Custom{}", CustomDataIndex).c_str()))
						{
							SetCustomData(CustomDataIndex, InstanceIndex, Data, true);
						}
					}
				}
			}
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
			AssetHandle<StaticMesh> MeshAsset(NewPath);
			MeshAsset.Load();

			SetMesh(MeshAsset);
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