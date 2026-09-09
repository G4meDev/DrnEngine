#include "DrnPCH.h"
#include "AssetPreviewSkeletalMeshGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"
#include "Editor/FileImportMenu/FileImportMenu.h"
#include "Editor/EditorPanels/ViewportPanel.h"

#include "Editor/ContentBrowser/ContentBrowser.h"

namespace Drn
{
	AssetPreviewSkeletalMeshGuiLayer::AssetPreviewSkeletalMeshGuiLayer(SkeletalMesh* InOwningAsset)
		: m_ShowSceneSetting(true)
		, m_ShowDetail(true)
		, m_DebugLinesSize(0.1f)
		, m_DrawNormals(false)
		, m_DrawTangents(false)
		, m_DrawBitTangents(false)
		, m_DrawBounds(false)
		, m_PreviewWeights(false)
		, SelectedBoneIndex(-1)
	{
		//LOG(LogSkeletalMeshPreview, Info, "opening %s", InOwningAsset->m_Path.c_str());

		m_OwningAsset = AssetHandle<SkeletalMesh>(InOwningAsset->m_Path);
		m_OwningAsset.Load();

		PreviewWorld = WorldManager::Get()->AllocateWorld();
		PreviewWorld->SetTransient(true);
		PreviewWorld->SetPaused(true);
		PreviewWorld->SetEditorWorld();

		PreviewMesh = PreviewWorld->SpawnActor<SkeletalMeshActor>();
		//PreviewMesh->GetMeshComponent()->SetSelectable(false);
		PreviewMesh->GetMeshComponent()->SetMesh(m_OwningAsset);
		PreviewMesh->GetMeshComponent()->SetTickInEditor(true);

		m_SkyLight = PreviewWorld->SpawnActor<SkyLightActor>();
		m_SkyLight->SetIntensity(0.4f);

		m_DirectionalLight = PreviewWorld->SpawnActor<DirectionalLightActor>();
		m_DirectionalLight->SetIntensity(1);
		m_DirectionalLight->SetActorRotation(Quat(0, XM_PIDIV4, XM_PI));

		m_ViewportPanel = std::make_unique<ViewportPanel>(PreviewWorld->GetScene());
		m_ViewportPanel->OnSelectedNewComponent.Add( this, &AssetPreviewSkeletalMeshGuiLayer::OnSelectedNewComponent );
		m_ViewportPanel->GetGizmoTransformDel.Bind( this, &AssetPreviewSkeletalMeshGuiLayer::GetGizmoTransform );
		m_ViewportPanel->OnGizmoTransformChangedDel.Bind( this, &AssetPreviewSkeletalMeshGuiLayer::OnGizmoTransformChanged );

		AssetHandle<Material> BoneWeightPreviewMaterial("Engine\\Content\\Materials\\M_SkeletalMeshWeightPreview.drn");
		BoneWeightPreviewMaterial.Load();
		BoneWeightMaterial = MaterialInstanceDynamic::Create(BoneWeightPreviewMaterial);

		TRefCountPtr<AnimatorSkeletalMeshPreview> NewAnimator = new AnimatorSkeletalMeshPreview(this);
		PreviewMesh->GetMeshComponent()->SetAnimator(NewAnimator);

		Vector FocalPoint = m_OwningAsset->GetBounds().Origin;
		Vector CameraLocation = FocalPoint + Vector::OneVector * m_OwningAsset->GetBounds().SphereRadius * m_OwningAsset->ThumbnailDistance;
		PreviewWorld->GetViewportCamera()->SetActorTransform(Transform(CameraLocation, Quat::LookAtRotation(CameraLocation, FocalPoint)));

		OnReimport();
	}

	AssetPreviewSkeletalMeshGuiLayer::~AssetPreviewSkeletalMeshGuiLayer()
	{
		//LOG(LogSkeletalMeshPreview, Info, "closing %s", m_OwningAsset->m_Path.c_str());

		m_ViewportPanel->OnSelectedNewComponent.Clear();
		m_ViewportPanel->GetGizmoTransformDel.Unbind();
		m_ViewportPanel->OnGizmoTransformChangedDel.Unbind();

		if (PreviewWorld)
		{
			PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewSkeletalMeshGuiLayer::OnReimport()
	{
		for (int32 MaterialIndex = 0; MaterialIndex < PreviewMesh->GetMeshComponent()->GetMaterialCount(); MaterialIndex++)
		{
			PreviewMesh->GetMeshComponent()->SetMaterial(MaterialIndex, BoneWeightMaterial);
		}
		
		SelectedBoneIndex = -1;

		const int32 BoneCount = m_OwningAsset->Data.RefSkeleton.BonePose.size();
		BonePreviewTransforms.resize(BoneCount);
		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			BonePreviewTransforms[BoneIndex] = m_OwningAsset->Data.RefSkeleton.GetParentBoneSpaceTransform(BoneIndex);
			//BonePreviewTransforms[BoneIndex] = Matrix(m_OwningAsset->Data.RefSkeleton.BonePose[BoneIndex]).Inverse();
			//
			//const int32 ParentIndex = m_OwningAsset->Data.RefSkeleton.BoneInfo[BoneIndex].ParentIndex;
			//if (ParentIndex != -1)
			//{
			//	BonePreviewTransforms[BoneIndex] = BonePreviewTransforms[BoneIndex].GetRelativeTransform(Matrix(m_OwningAsset->Data.RefSkeleton.BonePose[ParentIndex]).Inverse());
			//}
		}
	}

	void AssetPreviewSkeletalMeshGuiLayer::Draw( float DeltaTime )
	{
		SCOPE_STAT();

		BoneWeightMaterial->SetNamedScalar("BoneIndex", SelectedBoneIndex);

		for (int32 MaterialIndex = 0; MaterialIndex < PreviewMesh->GetMeshComponent()->GetMaterialCount(); MaterialIndex++)
		{
			PreviewMesh->GetMeshComponent()->SetMaterialOverride(MaterialIndex, m_PreviewWeights);
		}

		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
			m_ViewportPanel->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

		DrawDebugs();

		DrawMenu();

		ImVec2 Size = ImGui::GetContentRegionAvail();
		float BorderSize = ImGui::GetStyle().FramePadding.x;

		bool bLeftPanel = m_ShowSceneSetting;
		bool bRightPanel = m_ShowDetail;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize) * (bLeftPanel + bRightPanel) , 0.0f );

		if ( m_ShowSceneSetting && ImGui::BeginChild( "Scene Setting", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			DrawSkeletonTree();
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			m_ViewportPanel->SetRenderingEnabled(true);
			m_ViewportPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (m_ShowDetail && ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			DrawDetailPanel();

		}
		ImGui::EndChild();


		ImGui::End();
	}

	void AssetPreviewSkeletalMeshGuiLayer::DrawMenu()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
			if ( ImGui::BeginMenu( "File" ) )
			{
				ImGui::MenuItem( "nothing" );
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Window" ))
			{
				ImGui::MenuItem( "Scene Setting", NULL, &m_ShowSceneSetting);
				ImGui::MenuItem( "Detail", NULL, &m_ShowDetail);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "View" ))
			{
				auto DrawViewFlag = [&](EWorldViewFlag Flag, const char* Name)
				{
					bool ViewHasFlag = PreviewWorld->HasViewFlag(Flag);
					if (ImGui::MenuItem( Name, NULL, &ViewHasFlag))
					{
						PreviewWorld->SetViewFlag(Flag, ViewHasFlag);
					}
				};

				DrawViewFlag( EWorldViewFlag::Collision, "Collision" );

				ImGui::EndMenu();
			}

			if ( ImGui::BeginMenu( "Debug" ) )
			{
				if ( ImGui::MenuItem( "log live assets" ) )
				{
					AssetManager::Get()->ReportLiveAssets();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void AssetPreviewSkeletalMeshGuiLayer::DrawDetailPanel()
	{
		if (ImGui::Button( "save" ))
		{
			m_OwningAsset.Get()->Save();
		}

		ImGui::Separator();

		ImGui::TextWrapped("source file: %s", m_OwningAsset.Get()->m_SourcePath != NAME_NULL ? m_OwningAsset.Get()->m_SourcePath.c_str() : "...");
		
		if (ImGui::Button("reimport"))
		{
			m_OwningAsset.Get()->Import();
		}

		if ( ImGui::Button( "select" ) )
		{
			ShowSourceFileSelection();
		}

		if (ImGui::Button("import animation"))
		{
			ShowAnimationFileSelection();
		}

		ImGui::Separator();

		uint64 VertexCount = 0;
		for (auto& MeshSlot : m_OwningAsset->Data.MeshesData)
		{
			VertexCount += MeshSlot.VertexData.GetVertexCount();
		}

		ImGui::Text( "%u vertecies", VertexCount);

		ImGui::Separator();

		ImGui::Separator();
		ImGui::Text("Materials");
		
		for (MaterialProperty& Mat : m_OwningAsset->Data.Materials)
		{
			ImGui::Text(Mat.m_Name.c_str());
			ImGui::Text(Mat.GetMaterialPath().c_str());

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
				{
					auto AssetPath = static_cast<const char*>(payload->Data);
					AssetHandle<Asset> DropedMaterial(AssetPath);
					EAssetType Type = DropedMaterial.LoadGeneric();
					
					if (Type == EAssetType::Material)
					{
						Mat.SetMaterial(AssetHandle<Material>(AssetPath));
						PreviewMesh->GetMeshComponent()->MarkRenderStateDirty();
					}
					
					else if (Type == EAssetType::MaterialInstance)
					{
						Mat.SetMaterial(AssetHandle<MaterialInstance>(AssetPath));
						PreviewMesh->GetMeshComponent()->MarkRenderStateDirty();
					}
				}

				ImGui::EndDragDropTarget();
			}
		}

		ImGui::Separator();
		ImGui::InputFloat( "ImportScale", &m_OwningAsset.Get()->ImportScale);
		ImGui::Separator();

		//m_OwningAsset->m_BodySetup.DrawDetailPanel();

// ------------------------------------------------------------------------------------------------------

		//ImGui::Checkbox( "Use Complex Collision", &m_OwningAsset->m_BodySetup.m_UseTriMesh );
		ImGui::Separator();

// ------------------------------------------------------------------------------------------------------

		ImGui::Checkbox( "Import Normal", &m_OwningAsset->m_ImportNormals);
		ImGui::Checkbox( "Import Tangent", &m_OwningAsset->m_ImportTangents);
		ImGui::Checkbox( "Import Color", &m_OwningAsset->m_ImportColor);
		int32 UvNum = m_OwningAsset->m_ImportUVs;
		ImGui::SliderInt( "Import UVs", &UvNum, 0, 8);
		m_OwningAsset->m_ImportUVs = UvNum;

		ImGui::Separator();

// ------------------------------------------------------------------------------------------------------

		ImGui::Checkbox( "Draw Normals", &m_DrawNormals);
		ImGui::Checkbox( "Draw Tangent", &m_DrawTangents);
		ImGui::Checkbox( "Draw BitTangents", &m_DrawBitTangents);
		ImGui::DragFloat( "Line Size", &m_DebugLinesSize, 0.001, 10);
// ------------------------------------------------------------------------------------------------------

		ImGui::Separator();
		ImGui::Checkbox( "Draw Bounds", &m_DrawBounds);

		float PosBound[3] = {m_OwningAsset->PositiveBoundExtention.GetX(), m_OwningAsset->PositiveBoundExtention.GetY(), m_OwningAsset->PositiveBoundExtention.GetZ()};
		if (ImGui::InputFloat3("Positive Bound Extension", PosBound))
		{
			m_OwningAsset->PositiveBoundExtention = Vector(PosBound[0], PosBound[1], PosBound[2]);
		}

		float NegBound[3] = {m_OwningAsset->NegativeBoundExtention.GetX(), m_OwningAsset->NegativeBoundExtention.GetY(), m_OwningAsset->NegativeBoundExtention.GetZ()};
		if (ImGui::InputFloat3("Negative Bound Extension", NegBound))
		{
			m_OwningAsset->NegativeBoundExtention = Vector(NegBound[0], NegBound[1], NegBound[2]);
		}

// ------------------------------------------------------------------------------------------------------

		ImGui::Checkbox( "Preview Weights", &m_PreviewWeights);

		ImGui::Separator();
		if (SelectedBoneIndex != -1)
		{
			ImGui::Text(m_OwningAsset->Data.RefSkeleton.BoneInfo[SelectedBoneIndex].Name.c_str());
			//m_OwningAsset->Data.RefSkeleton.BonePose[SelectedBoneIndex].Draw("Selected Bone");
			BonePreviewTransforms[SelectedBoneIndex].Draw("Selected Bone");
		}

		ImGui::InputFloat("Thumbnail Distance", &m_OwningAsset->ThumbnailDistance);
	}

	void AssetPreviewSkeletalMeshGuiLayer::ShowSourceFileSelection()
	{
		Editor::Get()->OpenImportMenu(
			"Select source file", FileImportMenu::FileFilter_Any(),
			std::bind( &AssetPreviewSkeletalMeshGuiLayer::OnSelectedSourceFile, this, std::placeholders::_1 ) );
	}

	void AssetPreviewSkeletalMeshGuiLayer::OnSelectedSourceFile( std::string FilePath )
	{
		m_OwningAsset->m_SourcePath = FilePath;
		m_OwningAsset->Import();
	}

	void AssetPreviewSkeletalMeshGuiLayer::ShowAnimationFileSelection()
	{
		Editor::Get()->OpenImportMenu(
			"Select animation file", FileImportMenu::FileFilter_Model(),
			std::bind( &AssetPreviewSkeletalMeshGuiLayer::OnSelectedAnimationFile, this, std::placeholders::_1 ) );
	}

	void AssetPreviewSkeletalMeshGuiLayer::OnSelectedAnimationFile( std::string FilePath )
	{
		AssetImportUserData UserData;
		UserData.AssetType = EAssetType::AnimationSequence;
		UserData.UserData << m_OwningAsset.GetPath();
		UserData.UserData.Seek(0);

		AssetManager::Get()->Create(FilePath, ContentBrowser::Get()->GetSelectedFolderPath(), UserData);

		ContentBrowser::Get()->Refresh();
	}

	void AssetPreviewSkeletalMeshGuiLayer::DrawSkeletonTree()
	{
		DrawSkeletonTreeNode(0);
	}

	void AssetPreviewSkeletalMeshGuiLayer::DrawSkeletonTreeNode( int32 NodeIndex )
	{
		std::vector<MeshBoneInfo>& Bones = m_OwningAsset->Data.RefSkeleton.BoneInfo;

		if (NodeIndex >= Bones.size())
			return;

		const bool bSelected = SelectedBoneIndex == NodeIndex;
		ImGuiTreeNodeFlags_ NodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
		EnumAddFlags(NodeFlags, ImGuiTreeNodeFlags_OpenOnArrow);
		EnumAddFlags(NodeFlags, m_OwningAsset->Data.RefSkeleton.IsLeafBone(NodeIndex) ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None);
		EnumAddFlags(NodeFlags, bSelected ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None);

		if (ImGui::TreeNodeEx(Bones[NodeIndex].Name.c_str(), NodeFlags))
		{
			if (ImGui::IsItemClicked())
			{
				SelectedBoneIndex = NodeIndex;
			}

			for (int32 ChildBoneIndex = 0; ChildBoneIndex < Bones.size(); ChildBoneIndex++)
			{
				if (Bones[ChildBoneIndex].ParentIndex == NodeIndex)
				{
					DrawSkeletonTreeNode(ChildBoneIndex);
				}
			}

			ImGui::TreePop();
		}
	}

	void AssetPreviewSkeletalMeshGuiLayer::OnSelectedNewComponent( const HitProxyData& Data )
	{
		if (Data.ActorID == PreviewMesh->GetUniqueID() && Data.ComponentID == PreviewMesh->GetMeshComponent()->GetUniqueID())
		{
			SelectedBoneIndex = Data.CustomA;
		}
	}

	void AssetPreviewSkeletalMeshGuiLayer::GetGizmoTransform( bool& bDrawGizmo, Transform& GizmoTransform )
	{
		bDrawGizmo = SelectedBoneIndex != -1;
		if (bDrawGizmo)
		{
			ReferenceSkeleton& RefSkeleton = m_OwningAsset->Data.RefSkeleton;

			GizmoTransform = BonePreviewTransforms[SelectedBoneIndex];
			int32 ParentIndex = RefSkeleton.BoneInfo[SelectedBoneIndex].ParentIndex;

			while (ParentIndex != -1)
			{
				GizmoTransform = GizmoTransform * BonePreviewTransforms[ParentIndex];
				ParentIndex = RefSkeleton.BoneInfo[ParentIndex].ParentIndex;
			}
		}
	}

	void AssetPreviewSkeletalMeshGuiLayer::OnGizmoTransformChanged( const Transform& GizmoTransform, EGizmoSpace GizmoSpace )
	{
		if (SelectedBoneIndex != -1)
		{
			ReferenceSkeleton& RefSkeleton = m_OwningAsset->Data.RefSkeleton;
			int32 ParentIndex = m_OwningAsset->Data.RefSkeleton.BoneInfo[SelectedBoneIndex].ParentIndex;

			if (ParentIndex == -1)
			{
				BonePreviewTransforms[SelectedBoneIndex] = GizmoTransform;
			}
			else
			{
				Transform ParentWorldTransform = Transform::Identity;

				while (ParentIndex != -1)
				{
					ParentWorldTransform = ParentWorldTransform * BonePreviewTransforms[ParentIndex];
					ParentIndex = RefSkeleton.BoneInfo[ParentIndex].ParentIndex;
				}

				BonePreviewTransforms[SelectedBoneIndex] = GizmoTransform.GetRelativeTransform(ParentWorldTransform);
				//BonePreviewTransforms[SelectedBoneIndex] = GizmoTransform.GetRelativeTransform(Matrix(m_OwningAsset->Data.RefSkeleton.BonePose[ParentIndex]).Inverse());
			}
		}
	}

	void AssetPreviewSkeletalMeshGuiLayer::DrawDebugs()
	{
		if (m_DrawNormals || m_DrawTangents || m_DrawBitTangents)
		{
			for ( SkeletalMeshSlotData& Data : m_OwningAsset->Data.MeshesData )
			{
				for (uint64 i = 0; i < Data.VertexData.GetPositions().size(); i++)
				{
					const Vector& Pos = Data.VertexData.GetPositions()[i];

					if (m_DrawNormals && Data.VertexData.HasNormals())
					{
						const Vector& Normal = Math::UnpackUint32ToSignedNormalizedVector(Data.VertexData.GetNormals()[i]);
						PreviewWorld->DrawDebugLine( Pos, Pos + Normal * m_DebugLinesSize, Color::Green, 0, 0 );
					}

					if (m_DrawTangents && Data.VertexData.HasTangents())
					{
						const Vector& Tangent = Math::UnpackUint32ToSignedNormalizedVector(Data.VertexData.GetTangents()[i]);
						PreviewWorld->DrawDebugLine( Pos, Pos + Tangent * m_DebugLinesSize, Color::Blue, 0, 0 );
					}

					if (m_DrawBitTangents && Data.VertexData.HasNormals() && Data.VertexData.HasTangents())
					{
						const Vector& Normal = Math::UnpackUint32ToSignedNormalizedVector(Data.VertexData.GetNormals()[i]);
						const Vector& Tangent = Math::UnpackUint32ToSignedNormalizedVector(Data.VertexData.GetTangents()[i]);

						const Vector& BitTangent = Vector::CrossProduct(Tangent, Normal).GetSafeNormal();
						PreviewWorld->DrawDebugLine( Pos, Pos + BitTangent * m_DebugLinesSize, Color::Red, 0, 0 );
					}
				}
			}
		}

		if (m_DrawBounds)
		{
			const BoxSphereBounds& Bounds = m_OwningAsset->GetBounds();

			PreviewWorld->DrawDebugSphere(Bounds.Origin, Quat::Identity, Color::Yellow, Bounds.SphereRadius, 32, 0, 0);
			PreviewWorld->DrawDebugBox(Box(Bounds.BoxExtent * -1, Bounds.BoxExtent), Transform(Bounds.Origin, Quat::Identity), Color::Blue, 0, 0);
		}

		//for (auto& Mesh : m_OwningAsset->Data.MeshesData)
		//{
		//	for (int32 TriangleIndex = 0; TriangleIndex < Mesh.VertexData.GetIndexCount(); TriangleIndex+=3)
		//	{
		//		uint32 Pt0 = Mesh.VertexData.Use4BitIndices() ? Mesh.VertexData.GetIndices_32()[TriangleIndex + 0] : Mesh.VertexData.GetIndices_16()[TriangleIndex + 0];
		//		uint32 Pt1 = Mesh.VertexData.Use4BitIndices() ? Mesh.VertexData.GetIndices_32()[TriangleIndex + 1] : Mesh.VertexData.GetIndices_16()[TriangleIndex + 1];
		//		uint32 Pt2 = Mesh.VertexData.Use4BitIndices() ? Mesh.VertexData.GetIndices_32()[TriangleIndex + 2] : Mesh.VertexData.GetIndices_16()[TriangleIndex + 2];
		//
		//		PreviewWorld->DrawDebugLine(Mesh.VertexData.GetPositions()[Pt0], Mesh.VertexData.GetPositions()[Pt1], Color::White, 0, 0);
		//		PreviewWorld->DrawDebugLine(Mesh.VertexData.GetPositions()[Pt1], Mesh.VertexData.GetPositions()[Pt2], Color::White, 0, 0);
		//		PreviewWorld->DrawDebugLine(Mesh.VertexData.GetPositions()[Pt2], Mesh.VertexData.GetPositions()[Pt0], Color::White, 0, 0);
		//	}
		//}

		//if (true)
		//{
		//	for (SphereElem& Elem : m_OwningAsset->GetBodySetup()->m_AggGeo.SphereElems)
		//	{
		//		PreviewWorld->DrawDebugSphere(Elem.Center, Quat::Identity, Color::White, Elem.Radius, 32, 0.0f, 0.0f);
		//	}
		//	 
		//	for (BoxElem& Elem : m_OwningAsset->GetBodySetup()->m_AggGeo.BoxElems)
		//	{
		//		PreviewWorld->DrawDebugBox(Box(Elem.Extent * -1, Elem.Extent), Transform(Elem.Center, Elem.Rotation), Color::White, 0.0f, 0.0f);
		//	}
		//}
	}

	void AssetPreviewSkeletalMeshGuiLayer::SetCurrentFocus()
	{
		
	}

        }

#endif