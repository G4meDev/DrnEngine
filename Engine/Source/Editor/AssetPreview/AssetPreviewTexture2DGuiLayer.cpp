#include "DrnPCH.h"
#include "AssetPreviewTexture2DGuiLayer.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"

namespace Drn
{
	AssetPreviewTexture2DGuiLayer::AssetPreviewTexture2DGuiLayer( Texture2D* InOwningAsset )
	{
		m_OwningAsset = AssetHandle<Texture2D>( InOwningAsset->m_Path );
		m_OwningAsset.Load();
	}

	AssetPreviewTexture2DGuiLayer::~AssetPreviewTexture2DGuiLayer()
	{
		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewTexture2DGuiLayer::Draw( float DeltaTime )
	{
		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
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
			ImGui::Text("eraeraer");
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
		
	}

	void AssetPreviewTexture2DGuiLayer::DrawDetailsPanel()
	{
		ImGui::Text("aeraerare");
	}

}

#endif