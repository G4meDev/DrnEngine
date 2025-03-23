#include "DrnPCH.h"
#include "ContentBrowserGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/D3D12Scene.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"

#include "Editor/Editor.h"

#include "Runtime/Misc/FileSystem.h"

LOG_DEFINE_CATEGORY( LogContentBrowser, "ContentBrowser" );

namespace Drn
{
	ContentBrowserGuiLayer::ContentBrowserGuiLayer()
	{
		OnRefresh();
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
			ImGui::BeginGroup();

			if (RootFolder)
			{
				if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					DrawNextFolder(RootFolder.get());
					ImGui::EndTable();
				}
			}

			ImGui::EndGroup();
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

	void ContentBrowserGuiLayer::DrawNextFolder( SystemFileNode* Node )
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID( Node->File.m_FullPath.c_str() );

		ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
		tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick;
		tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

		if ( Node->Childs.size() == 0 )
		tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

		bool node_open = ImGui::TreeNodeEx( "", tree_flags, "%s", Node->File.m_ShortPath.c_str());

		if ( node_open )
		{
			for (SystemFileNode* child : Node->Childs)
			{
				if (child->File.m_IsDirectory)
				{
					DrawNextFolder( child );
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void ContentBrowserGuiLayer::OnImport()
	{
		LOG(LogContentBrowser, Info, "Import");


	}

	void ContentBrowserGuiLayer::OnRefresh()
	{
		LOG(LogContentBrowser, Info, "Refresh");
		
		FileSystem::GetFilesInDirectory("C:\\SelfProjects\\DrnEngine\\Content", RootFolder);
	}
}

#endif