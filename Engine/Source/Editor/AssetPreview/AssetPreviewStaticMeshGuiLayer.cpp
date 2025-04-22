#include "DrnPCH.h"
#include "AssetPreviewStaticMeshGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Editor/Editor.h"
#include "Editor/FileImportMenu/FileImportMenu.h"
#include "Editor/EditorPanels/ViewportPanel.h"

LOG_DEFINE_CATEGORY( LogStaticMeshPreview, "StaticMeshPreview" );

namespace Drn
{
	AssetPreviewStaticMeshGuiLayer::AssetPreviewStaticMeshGuiLayer(StaticMesh* InOwningAsset)
		: m_ShowSceneSetting(true)
		, m_ShowDetail(true)
	{
		LOG(LogStaticMeshPreview, Info, "opening %s", InOwningAsset->m_Path.c_str());

		m_OwningAsset = AssetHandle<StaticMesh>(InOwningAsset->m_Path);
		m_OwningAsset.Load();

		PreviewWorld = WorldManager::Get()->AllocateWorld();

		PreviewMesh = PreviewWorld->SpawnActor<StaticMeshActor>();
		PreviewMesh->GetMeshComponent()->SetMesh(m_OwningAsset);

		m_ViewportPanel = std::make_unique<ViewportPanel>(PreviewWorld->GetScene());
	}

	AssetPreviewStaticMeshGuiLayer::~AssetPreviewStaticMeshGuiLayer()
	{
		LOG(LogStaticMeshPreview, Info, "closing %s", m_OwningAsset->m_Path.c_str());

		if (WorldManager::Get() && PreviewWorld)
		{
			WorldManager::Get()->ReleaseWorld(PreviewWorld);
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewStaticMeshGuiLayer::Draw( float DeltaTime )
	{
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

			ImGui::EndChild();
		}

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

			ImGui::EndChild();
		}


		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::DrawMenu()
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

	void AssetPreviewStaticMeshGuiLayer::DrawDetailPanel()
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

		ImGui::Separator();

		ImGui::InputFloat( "ImportScale", &m_OwningAsset.Get()->ImportScale);

		ImGui::Separator();

		m_OwningAsset->m_BodySetup.DrawDetailPanel();
	}

	void AssetPreviewStaticMeshGuiLayer::ShowSourceFileSelection()
	{
		Editor::Get()->OpenImportMenu(
			"Select source file", FileImportMenu::FileFilter_Any(),
			std::bind( &AssetPreviewStaticMeshGuiLayer::OnSelectedSourceFile, this, std::placeholders::_1 ) );
	}

	void AssetPreviewStaticMeshGuiLayer::OnSelectedSourceFile( std::string FilePath )
	{
		m_OwningAsset->m_SourcePath = FilePath;
		m_OwningAsset->Import();
	}

	void AssetPreviewStaticMeshGuiLayer::SetCurrentFocus()
	{
		
	}

}

#endif