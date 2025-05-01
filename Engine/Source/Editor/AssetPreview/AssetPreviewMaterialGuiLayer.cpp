#include "DrnPCH.h"
#include "AssetPreviewMaterialGuiLayer.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/Editor.h"

LOG_DEFINE_CATEGORY( LogAssetPreviewMaterial, "AssetPreviewMaterial" );

namespace Drn
{
	AssetPreviewMaterialGuiLayer::AssetPreviewMaterialGuiLayer( Material* InOwningAsset )
		: m_ShowDetailsPanel(true)
		, m_ShowSceneSettingsPanel(true)
	{
		LOG( LogAssetPreviewMaterial, Info, "opening %s", InOwningAsset->m_Path.c_str() );

		m_OwningAsset = AssetHandle<Material>( InOwningAsset->m_Path );
		m_OwningAsset.Load();

		PreviewWorld = WorldManager::Get()->AllocateWorld();

		//AssetHandle<StaticMesh> SphereMesh( "Engine\\Content\\BasicShapes\\SM_Sphere.drn" );
		//SphereMesh.Load();
		//
		//PreviewMesh = PreviewWorld->SpawnActor<StaticMeshActor>();
		//PreviewMesh->GetMeshComponent()->SetMesh( SphereMesh );

		m_ViewportPanel = std::make_unique<ViewportPanel>( PreviewWorld->GetScene() );
	}

	AssetPreviewMaterialGuiLayer::~AssetPreviewMaterialGuiLayer()
	{
		LOG(LogAssetPreviewMaterial, Info, "closing %s", m_OwningAsset->m_Path.c_str());

		if (PreviewWorld)
		{
			PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewMaterialGuiLayer::Draw( float DeltaTime )
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

		bool bLeftPanel = m_ShowSceneSettingsPanel;
		bool bRightPanel = m_ShowDetailsPanel;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize) * (bLeftPanel + bRightPanel) , 0.0f );

		if ( m_ShowSceneSettingsPanel && ImGui::BeginChild( "Scene Setting", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
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
		if (m_ShowDetailsPanel && ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			DrawDetailsPanel();

			ImGui::EndChild();
		}


		ImGui::End();
	}

	void AssetPreviewMaterialGuiLayer::DrawMenu()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
			if (ImGui::BeginMenu( "Window" ))
			{
				ImGui::MenuItem( "Scene Setting", NULL, &m_ShowSceneSettingsPanel);
				ImGui::MenuItem( "Detail", NULL, &m_ShowDetailsPanel);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void AssetPreviewMaterialGuiLayer::DrawDetailsPanel()
	{
		if (ImGui::Button("Import"))
		{
			m_OwningAsset->Import();
		}

		const char* PrimitiveTypes[] = { "Point", "Line", "Triangle", "Patch" };
		int CurrentType = static_cast<int>(m_OwningAsset->m_PrimitiveType) - 1;
		if ( ImGui::Combo( "Primitive Type", &CurrentType, PrimitiveTypes, IM_ARRAYSIZE( PrimitiveTypes )))
		{
			m_OwningAsset->m_PrimitiveType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(CurrentType + 1);
		}

		const char* LayoutTypes[] = { "Color", "LineColorThickness", "StandardMesh" };
		CurrentType = static_cast<int>(m_OwningAsset->m_InputLayoutType);
		if ( ImGui::Combo( "Layout Type", &CurrentType, LayoutTypes, IM_ARRAYSIZE( LayoutTypes )))
		{
			m_OwningAsset->m_InputLayoutType = static_cast<EInputLayoutType>(CurrentType);
		}

		const char* CullModes[] = { "None", "Front", "Back" };
		CurrentType = static_cast<int>(m_OwningAsset->m_CullMode) - 1;
		if ( ImGui::Combo( "Cull Mode", &CurrentType, CullModes, IM_ARRAYSIZE( CullModes )))
		{
			m_OwningAsset->m_CullMode = static_cast<D3D12_CULL_MODE>(CurrentType + 1);
		}
	}

}

#endif