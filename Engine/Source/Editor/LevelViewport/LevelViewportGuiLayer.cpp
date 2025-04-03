#include "DrnPCH.h"
#include "LevelViewportGuiLayer.h"

#if WITH_EDITOR

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/EditorPanels/WorldOutlinerPanel.h"
#include "Editor/EditorPanels/ActorDetailPanel.h"
#include <imgui.h>

namespace Drn
{
	LevelViewportGuiLayer::LevelViewportGuiLayer()
		: m_ShowOutliner(true)
		, m_ShowDetail(true)
	{
		m_ViewportPanel = std::make_unique<ViewportPanel>( Renderer::Get()->m_MainScene );
		m_WorldOutlinerPanel = std::make_unique<WorldOutlinerPanel>(WorldManager::Get()->GetMainWorld() );
		m_ActorDetailPanel = std::make_unique<ActorDetailPanel>();
	}

	LevelViewportGuiLayer::~LevelViewportGuiLayer()
	{
		
	}

	void LevelViewportGuiLayer::Draw( float DeltaTime )
	{
		if (!ImGui::Begin("Level Viewport"))
		{
			m_ViewportPanel->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

		DrawMenuBar(DeltaTime);

		ImVec2 Size = ImGui::GetContentRegionAvail();
		float BorderSize = ImGui::GetStyle().FramePadding.x;

		bool bLeftPanel = m_ShowOutliner;
		bool bRightPanel = m_ShowDetail;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize) * (bLeftPanel + bRightPanel) , 0.0f );

		if ( m_ShowOutliner && ImGui::BeginChild( "World Outliner", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			m_WorldOutlinerPanel->Draw(DeltaTime);

			ImGui::EndChild();
		}

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			m_ViewportPanel->SetRenderingEnabled(true);
			m_ViewportPanel->Draw(DeltaTime);

			if ( ImGui::BeginDragDropTarget() )
			{
				if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
				{
					HandleViewportPayload(Payload);
				}

				ImGui::EndDragDropTarget();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (m_ShowDetail && ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			m_ActorDetailPanel->SetSelectedActor(m_WorldOutlinerPanel->GetSelectedActor());
			m_ActorDetailPanel->Draw(DeltaTime);

			ImGui::EndChild();
		}

		ImGui::End();
	}

	void LevelViewportGuiLayer::DrawMenuBar( float DeltaTime )
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu( "File" ))
			{
				ImGui::MenuItem( "Nothing" );
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Window" ))
			{
				ImGui::MenuItem( "Outliner", NULL, &m_ShowOutliner);
				ImGui::MenuItem( "Detail", NULL, &m_ShowDetail);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Debug" ))
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

	void LevelViewportGuiLayer::HandleViewportPayload( const ImGuiPayload* Payload )
	{
		auto AssetPath = static_cast<const char*>(Payload->Data);

		AssetHandle<Asset> asset(AssetPath);
		EAssetType Type = asset.LoadGeneric();

		if (asset.IsValid())
		{
			if (Type == EAssetType::StaticMesh)
			{
				StaticMeshActor* NewActor = WorldManager::Get()->GetMainWorld()->SpawnActor<StaticMeshActor>();

				AssetHandle<StaticMesh> MeshAsset(AssetPath);
				MeshAsset.Load();
				NewActor->GetMeshComponent()->SetMesh(MeshAsset);
			}
		}
	}

}

#endif