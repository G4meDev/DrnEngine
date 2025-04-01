#include "DrnPCH.h"
#include "LevelViewportGuiLayer.h"

#if WITH_EDITOR

#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/EditorPanels/WorldOutlinerPanel.h"
#include <imgui.h>

namespace Drn
{
	LevelViewportGuiLayer::LevelViewportGuiLayer()
	{
		m_ViewportPanel = std::make_unique<ViewportPanel>();
		m_WorldOutlinerPanel = std::make_unique<WorldOutlinerPanel>();
	}

	LevelViewportGuiLayer::~LevelViewportGuiLayer()
	{
		
	}

	void LevelViewportGuiLayer::Draw( float DeltaTime )
	{
		if (!ImGui::Begin("Level Viewport"))
		{
			ImGui::End();
			return;
		}

		if ( ImGui::BeginChild( "World Outliner", ImVec2(300.0f, 0.0f), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			m_WorldOutlinerPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ImVec2(0.0f, 0.0f), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			m_ViewportPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Detail", ImVec2(300.0f, 0.0f), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{

		}
		ImGui::EndChild();


		ImGui::End();
	}

}

#endif