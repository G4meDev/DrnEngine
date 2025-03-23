#include "DrnPCH.h"
#include "ContentBrowserGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/D3D12Scene.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"

#include "Editor/Editor.h"

LOG_DEFINE_CATEGORY( LogContentBrowser, "ContentBrowser" );

namespace Drn
{
	ContentBrowserGuiLayer::ContentBrowserGuiLayer()
	{
		
	}
	
	ContentBrowserGuiLayer::~ContentBrowserGuiLayer()
	{
		
	}

	void ContentBrowserGuiLayer::Draw()
	{
		ImGui::Begin("ContentBrowser");

		if (ImGui::Button( "Import" ))
			OnImport();
		ImGui::SameLine();
		if (ImGui::Button( "Refresh" ))
			OnRefresh();

		if (ImGui::BeginChild("FolderView", ImVec2(300, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			//ImGui::SetNextItemWidth( -FLT_MIN );
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("FileView", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			
		}
		ImGui::EndChild();

		ImGui::End();

		ImGui::ShowDemoWindow();
	}

	void ContentBrowserGuiLayer::OnImport()
	{
		LOG(LogContentBrowser, Info, "Import");
	}

	void ContentBrowserGuiLayer::OnRefresh()
	{
		LOG(LogContentBrowser, Info, "Refresh");
	}
}

#endif