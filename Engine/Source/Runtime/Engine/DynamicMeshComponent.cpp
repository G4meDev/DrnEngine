#include "DrnPCH.h"
#include "DynamicMeshComponent.h"

#include "DrnPCH.h"
#include "DynamicMeshComponent.h"
#include "Runtime/Renderer/DynamicMeshSceneProxy.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#endif

namespace Drn
{
	DynamicMeshComponent::DynamicMeshComponent()
		: PrimitiveComponent()
		, m_DynamicMeshSceneProxy(nullptr)
		, MinDrawDistance(0)
		, MaxDrawDistance(0)
	{
		
	}

	DynamicMeshComponent::~DynamicMeshComponent()
	{
		
	}

	void DynamicMeshComponent::Tick( float DeltaTime )
	{
		PrimitiveComponent::Tick(DeltaTime);

	}

	void DynamicMeshComponent::Serialize( Archive& Ar )
	{
		PrimitiveComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			BufferArchive CompressedAr(10, true);
			Ar >> CompressedAr;
			CompressedAr.Decompress();
			static_cast<Archive*>(&CompressedAr)->operator>> <uint8>(MeshSecions);

			Ar >> MinDrawDistance;
			Ar >> MaxDrawDistance;

			CalculateLocalBounds();
			UpdateBounds();
		}

		else
		{
			BufferArchive CompressedAr(10, false);
			static_cast<Archive*>(&CompressedAr)->operator<< <uint8>(MeshSecions);
			CompressedAr.Compress();
			Ar << CompressedAr;

			Ar << MinDrawDistance;
			Ar << MaxDrawDistance;
		}
	}

	void DynamicMeshComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		CreatePhysicState();

		m_DynamicMeshSceneProxy = new DynamicMeshSceneProxy(this);
		InOwningWorld->GetScene()->RegisterPrimitiveProxy(m_DynamicMeshSceneProxy);
		m_SceneProxy = m_DynamicMeshSceneProxy;
	}

	void DynamicMeshComponent::UnRegisterComponent()
	{
		DestroyPhysicState();

		if (m_SceneProxy)
		{
			m_SceneProxy->MarkPendingKill();
			m_SceneProxy = nullptr;
			m_DynamicMeshSceneProxy = nullptr;
		}

		PrimitiveComponent::UnRegisterComponent();
	}

	void DynamicMeshComponent::OnUpdateTransform(bool SkipPhysic)
	{
		PrimitiveComponent::OnUpdateTransform(SkipPhysic);

		UpdateBounds();
	}

	void DynamicMeshComponent::CreatePhysicState()
	{
		//if (GetBodySetup() && GetBodySetup()->HasCollision())
		//{
		//	m_BodyInstance.InitBody(Mesh->GetBodySetup(), GetWorldTransform(), this, GetWorld()->GetPhysicScene());
		//	bPhysicStateCreated = true;
		//}
	}

	void DynamicMeshComponent::DestroyPhysicState()
	{
		//m_BodyInstance.TermBody();
		//bPhysicStateCreated = false;
	}

	void DynamicMeshComponent::RecreatePhysicState()
	{
		//drn_check(IsRegistered());
		//
		//DestroyPhysicState();
		//CreatePhysicState();
	}

	void DynamicMeshComponent::SetMaterial( uint16 MaterialIndex, AssetHandle<Material>& InMaterial )
	{
		if (MaterialIndex < MeshSecions.size())
		{
			MeshSecions[MaterialIndex].SectionMaterial.SetMaterial(InMaterial);
			MarkRenderStateDirty();
		}
	}

	void DynamicMeshComponent::SetMaterial( uint16 MaterialIndex, AssetHandle<MaterialInstance>& InMaterial )
	{
		if (MaterialIndex < MeshSecions.size())
		{
			MeshSecions[MaterialIndex].SectionMaterial.SetMaterial(InMaterial);
			MarkRenderStateDirty();
		}
	}

	void DynamicMeshComponent::SetMaterial( uint16 MaterialIndex, TRefCountPtr<MaterialInstanceDynamic> InMaterial )
	{
		if (MaterialIndex < MeshSecions.size())
		{
			MeshSecions[MaterialIndex].SectionMaterial.SetMaterial(InMaterial);
			MarkRenderStateDirty();
		}
	}

#if WITH_EDITOR

	void DynamicMeshComponent::DrawDetailPanel( float DeltaTime )
	{
		if (ImGui::Checkbox("Static", &bStatic))
		{
			SetStatic(bStatic);
		}

		PrimitiveComponent::DrawDetailPanel(DeltaTime);

		ImGui::TextWrapped( "Guid: %s", m_Guid.ToString().c_str());

		if (ImGui::InputFloat("MinDrawDistance", &MinDrawDistance))
		{
			SetMinDrawDistance(MinDrawDistance);
		}

		if (ImGui::InputFloat("MaxDrawDistance", &MaxDrawDistance))
		{
			SetMaxDrawDistance(MaxDrawDistance);
		}
	}

	bool DynamicMeshComponent::IsUsingMaterial( const AssetHandle<Material>& Mat )
	{
		for (const DynamicMeshSection& MeshSecion : MeshSecions)
		{
			if (MeshSecion.SectionMaterial.GetMaterialInterface() && MeshSecion.SectionMaterial.GetMaterialInterface()->IsDependent(*Mat))
			{
				return true;
			}
		}

		return false;
	}

	void DynamicMeshComponent::SetSelectedInEditor(bool SelectedInEditor, const HitProxyData& Data)
	{
		PrimitiveComponent::SetSelectedInEditor(SelectedInEditor, Data);
	
		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectedInEditor( SelectedInEditor );
		}
	}

	void DynamicMeshComponent::SetSelectable( bool Selectable )
	{
		PrimitiveComponent::SetSelectable(Selectable);

		if (m_SceneProxy)
		{
			m_SceneProxy->SetSelectable(Selectable);
		}
	}

	void DynamicMeshComponent::DrawEditorDefault()
	{
		PrimitiveComponent::DrawEditorDefault();
		
	}

	void DynamicMeshComponent::DrawEditorSelected()
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

	void DynamicMeshComponent::SetMinDrawDistance( float Value )
	{
		MinDrawDistance = Value;
		if (m_DynamicMeshSceneProxy)
		{
			m_DynamicMeshSceneProxy->MinDrawDistance = MinDrawDistance;
		}
	}

	void DynamicMeshComponent::SetMaxDrawDistance( float Value )
	{
		MaxDrawDistance = Value;
		if (m_DynamicMeshSceneProxy)
		{
			m_DynamicMeshSceneProxy->MaxDrawDistance = MaxDrawDistance;
		}
	}

	BoxSphereBounds DynamicMeshComponent::CalcBounds( const Transform& LocalToWorld ) const
	{
		return LocalBounds.TransformBy(LocalToWorld);
	}

	void DynamicMeshComponent::CreateMeshSection_Color( int32 SectionIndex, MaterialSlot InMaterial, const std::vector<Vector>& Positions, const std::vector<uint32>& Indices, const std::vector<Color>&  Colors )
	{
		CreateMeshSection(SectionIndex, InMaterial, Positions, Indices, {}, {}, Colors, {}, {}, {}, {});
	}

	void DynamicMeshComponent::CreateMeshSection( int32 SectionIndex, MaterialSlot InMaterial, const std::vector<Vector>& Positions, const std::vector<uint32>& Indices, const std::vector<Vector>& Normals, const std::vector<Vector>& Tangents,
		const std::vector<Color>& Colors, const std::vector<Vector2>& UV0, const std::vector<Vector2>& UV1, const std::vector<Vector2>& UV2, const std::vector<Vector2>& UV3 )
	{
		if (MeshSecions.size() <= SectionIndex)
		{
			MeshSecions.resize(SectionIndex + 1);
		}

		DynamicMeshSection& NewSection = MeshSecions[SectionIndex];
		NewSection.Reset();

		const int32 VertexCount = Positions.size();

		NewSection.VertexBufferData.Positions = Positions;

		if (Normals.size() > 0)
		{
			drn_check(Normals.size() == VertexCount);
			NewSection.VertexBufferData.Normals.resize(VertexCount);

			for (int32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
			{
				NewSection.VertexBufferData.Normals[VertexIndex] = Math::PackSignedNormalizedVectorToUint32(Normals[VertexIndex]);
			}
		}

		if (Tangents.size() > 0)
		{
			drn_check(Tangents.size() == VertexCount);
			NewSection.VertexBufferData.Tangents.resize(VertexCount);

			for (int32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
			{
				NewSection.VertexBufferData.Tangents[VertexIndex] = Math::PackSignedNormalizedVectorToUint32(Tangents[VertexIndex]);
			}
		}

		if (Colors.size() > 0)
		{
			drn_check(Colors.size() == VertexCount);
			NewSection.VertexBufferData.Colors = Colors;
		}

		auto CopyUVs = [&](std::vector<Vector2Half>& Target, const std::vector<Vector2>& Source)
		{
			if (Source.size() > 0)
			{
				drn_check(Source.size() == VertexCount);
				Target.resize(VertexCount);

				for (int32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
				{
					Target[VertexIndex] = Vector2Half(Source[VertexIndex].X, Source[VertexIndex].Y);
				}
			}
		};

		CopyUVs(NewSection.VertexBufferData.UV_1, UV0);
		CopyUVs(NewSection.VertexBufferData.UV_2, UV1);
		CopyUVs(NewSection.VertexBufferData.UV_3, UV2);
		CopyUVs(NewSection.VertexBufferData.UV_4, UV3);

		NewSection.VertexBufferData.bUse4BitIndices = true;
		NewSection.VertexBufferData.Indices_32 = Indices;

		NewSection.VertexBufferData.VertexCount = VertexCount;
		NewSection.VertexBufferData.IndexCount = Indices.size();

		NewSection.SectionMaterial = InMaterial;

		for (int32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			NewSection.SectionLocalBox += Positions[VertexIndex];
		}

		CalculateLocalBounds();
		UpdateBounds();
	}

	void DynamicMeshComponent::ClearSections()
	{
		MeshSecions.clear();

		CalculateLocalBounds();
		UpdateBounds();
	}

	void DynamicMeshComponent::CalculateLocalBounds()
	{
		Box LocalBox;
		LocalBox.Init();

		for (const DynamicMeshSection& Sec : MeshSecions)
		{
			LocalBox += Sec.SectionLocalBox;
		}

		LocalBounds = LocalBox;
	}

	class Archive& operator<<(Archive& Ar, const DynamicMeshSection& Value)
	{
		const_cast<StaticMeshVertexData*>(&Value.VertexBufferData)->Serialize(Ar);
		const_cast<MaterialSlot*>(&Value.SectionMaterial)->Serialize(Ar);
		Ar << Value.SectionLocalBox;
		Ar << Value.bEnableCollision;
		Ar << Value.bSectionVisible;

		return Ar;
	}

	class Archive& operator>>(Archive& Ar, DynamicMeshSection& Value)
	{
		Value.VertexBufferData.Serialize(Ar);
		Value.SectionMaterial.Serialize(Ar);
		Ar >> Value.SectionLocalBox;
		Ar >> Value.bEnableCollision;
		Ar >> Value.bSectionVisible;
		
		return Ar;
	}

        }  // namespace Drn