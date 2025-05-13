#include "DrnPCH.h"
#include "AssetPreviewTexture2DGuiLayer.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	AssetPreviewTexture2DGuiLayer::AssetPreviewTexture2DGuiLayer( Texture2D* InOwningAsset )
	{
		m_OwningAsset = AssetHandle<Texture2D>( InOwningAsset->m_Path );
		m_OwningAsset.Load();

		m_PreviewWorld = WorldManager::Get()->AllocateWorld();

		AssetHandle<StaticMesh> PlaneMesh( "Engine\\Content\\BasicShapes\\SM_Quad.drn" );
		PlaneMesh.Load();
		
		AssetHandle<Material> Texture2DPreviewMaterial( "Engine\\Content\\Materials\\M_Texture2DPreview.drn" );
		Texture2DPreviewMaterial.Load();
		Texture2DPreviewMaterial->SetNamedTexture2D("Texture", m_OwningAsset);

		m_PreviewMeshPlane = m_PreviewWorld->SpawnActor<StaticMeshActor>();
		m_PreviewMeshPlane->GetMeshComponent()->SetMesh( PlaneMesh );
		m_PreviewMeshPlane->GetMeshComponent()->SetMaterial(0, Texture2DPreviewMaterial);

		m_ViewportPanel = std::make_unique<ViewportPanel>( m_PreviewWorld->GetScene() );
	}

	AssetPreviewTexture2DGuiLayer::~AssetPreviewTexture2DGuiLayer()
	{
		if (m_PreviewWorld)
		{
			m_PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}


	void AssetPreviewTexture2DGuiLayer::Draw( float DeltaTime )
	{
		SCOPE_STAT(Texture2DPreviewGuiLayerDraw);

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

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize), 0.0f );

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			m_ViewportPanel->SetRenderingEnabled(true);
			m_ViewportPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			DrawDetailsPanel();

			ImGui::EndChild();
		}


		ImGui::End();
	}

	void AssetPreviewTexture2DGuiLayer::DrawMenu()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
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

	void AssetPreviewTexture2DGuiLayer::DrawDetailsPanel()
	{
		ImGui::Text("Detail Panel");
	}

}

#endif