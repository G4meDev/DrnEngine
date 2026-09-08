#include "DrnPCH.h"
#include "AssetPreviewAnimationSequenceGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"
#include "Editor/FileImportMenu/FileImportMenu.h"
#include "Editor/EditorPanels/ViewportPanel.h"

namespace Drn
{
	AssetPreviewAnimationSequenceGuiLayer::AssetPreviewAnimationSequenceGuiLayer( AnimationSequence* InOwningAsset )
		: m_ShowSceneSetting(true)
		, m_ShowDetail(true)
		, SelectedBoneIndex(-1)
		, DisplayFrameNumber(-1)
	{
		m_OwningAsset = AssetHandle<AnimationSequence>(InOwningAsset->m_Path);
		m_OwningAsset.Load();

		PreviewWorld = WorldManager::Get()->AllocateWorld();
		PreviewWorld->SetTransient(true);
		PreviewWorld->SetPaused(true);
		PreviewWorld->SetEditorWorld();

		PreviewMesh = PreviewWorld->SpawnActor<SkeletalMeshActor>();
		PreviewMesh->GetMeshComponent()->SetMesh(m_OwningAsset->OwningSkeleton);
		PreviewMesh->GetMeshComponent()->SetTickInEditor(true);

		m_SkyLight = PreviewWorld->SpawnActor<SkyLightActor>();
		m_SkyLight->SetIntensity(0.4f);

		m_DirectionalLight = PreviewWorld->SpawnActor<DirectionalLightActor>();
		m_DirectionalLight->SetIntensity(1);
		m_DirectionalLight->SetActorRotation(Quat(0, XM_PIDIV4, XM_PI));

		m_ViewportPanel = std::make_unique<ViewportPanel>(PreviewWorld->GetScene());
		m_ViewportPanel->OnSelectedNewComponent.Add( this, &AssetPreviewAnimationSequenceGuiLayer::OnSelectedNewComponent );
		m_ViewportPanel->GetGizmoTransformDel.Bind( this, &AssetPreviewAnimationSequenceGuiLayer::GetGizmoTransform );
		m_ViewportPanel->OnGizmoTransformChangedDel.Bind( this, &AssetPreviewAnimationSequenceGuiLayer::OnGizmoTransformChanged );

		TRefCountPtr<AnimatorAnimationSequencePreview> NewAnimator = new AnimatorAnimationSequencePreview(this);
		PreviewMesh->GetMeshComponent()->SetAnimator(NewAnimator);

		OnReimport();
	}

	AssetPreviewAnimationSequenceGuiLayer::~AssetPreviewAnimationSequenceGuiLayer()
	{
		m_ViewportPanel->OnSelectedNewComponent.Clear();
		m_ViewportPanel->GetGizmoTransformDel.Unbind();
		m_ViewportPanel->OnGizmoTransformChangedDel.Unbind();

		if (PreviewWorld)
		{
			PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewAnimationSequenceGuiLayer::OnReimport()
	{
		SelectedBoneIndex = -1;
	}

	void AssetPreviewAnimationSequenceGuiLayer::Draw( float DeltaTime )
	{
		SCOPE_STAT();

		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
			m_ViewportPanel->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

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

	void AssetPreviewAnimationSequenceGuiLayer::DrawMenu()
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

	void AssetPreviewAnimationSequenceGuiLayer::DrawDetailPanel()
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


		ImGui::InputFloat("Preview Speed", &PreviewSpeed);
		ImGui::Checkbox("Step Animation", &StepAnimation);
		ImGui::InputInt("Frame Number", &DisplayFrameNumber);

		ImGui::Separator();

		AnimatorAnimationSequencePreview* Anim = dynamic_cast<AnimatorAnimationSequencePreview*>(PreviewMesh->GetMeshComponent()->GetAnimator());
		if (Anim)
		{
			std::string TimeDisplayInfo = std::format("{:.2f} / {:.2f}", Anim->GetCurrentTime(), m_OwningAsset->Data.Length);
			ImGui::Text(TimeDisplayInfo.c_str());

			std::string FrameDisplayInfo = std::format("{} / {}", Anim->GetCurrentFrame(), m_OwningAsset->Data.KeyFrames.size());
			ImGui::Text(FrameDisplayInfo.c_str());
		}
	}

	void AssetPreviewAnimationSequenceGuiLayer::ShowSourceFileSelection()
	{
		Editor::Get()->OpenImportMenu(
			"Select source file", FileImportMenu::FileFilter_Any(),
			std::bind( &AssetPreviewAnimationSequenceGuiLayer::OnSelectedSourceFile, this, std::placeholders::_1 ) );
	}

	void AssetPreviewAnimationSequenceGuiLayer::OnSelectedSourceFile( std::string FilePath )
	{
		m_OwningAsset->m_SourcePath = FilePath;
		m_OwningAsset->Import();
	}

	void AssetPreviewAnimationSequenceGuiLayer::DrawSkeletonTree()
	{
		DrawSkeletonTreeNode(0);
	}

	void AssetPreviewAnimationSequenceGuiLayer::DrawSkeletonTreeNode( int32 NodeIndex )
	{
		SkeletalMesh* Mesh = m_OwningAsset->OwningSkeleton.Get();
		const std::vector<MeshBoneInfo>& Bones = Mesh->GetData().RefSkeleton.BoneInfo;

		if (NodeIndex >= Bones.size())
			return;

		const bool bSelected = SelectedBoneIndex == NodeIndex;
		ImGuiTreeNodeFlags_ NodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
		EnumAddFlags(NodeFlags, ImGuiTreeNodeFlags_OpenOnArrow);
		EnumAddFlags(NodeFlags, Mesh->GetData().RefSkeleton.IsLeafBone(NodeIndex) ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None);
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

	void AssetPreviewAnimationSequenceGuiLayer::OnSelectedNewComponent( const HitProxyData& Data )
	{
		if (Data.ActorID == PreviewMesh->GetUniqueID() && Data.ComponentID == PreviewMesh->GetMeshComponent()->GetUniqueID())
		{
			SelectedBoneIndex = Data.CustomA;
		}
	}

	void AssetPreviewAnimationSequenceGuiLayer::GetGizmoTransform( bool& bDrawGizmo, Transform& GizmoTransform )
	{
		bDrawGizmo = SelectedBoneIndex != -1;
		if (bDrawGizmo && PreviewMesh->GetMeshComponent()->GetAnimator())
		{
			AnimatorAnimationSequencePreview* Anim = dynamic_cast<AnimatorAnimationSequencePreview*>(PreviewMesh->GetMeshComponent()->GetAnimator());
			GizmoTransform = Anim ? Anim->GetBoneWorldTransform(SelectedBoneIndex) : Transform::Identity;
		}
	}

	void AssetPreviewAnimationSequenceGuiLayer::OnGizmoTransformChanged( const Transform& GizmoTransform, EGizmoSpace GizmoSpace )
	{
		
	}

}  // namespace Drn

#endif