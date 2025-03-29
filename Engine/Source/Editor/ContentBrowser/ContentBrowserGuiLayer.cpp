#include "DrnPCH.h"
#include "ContentBrowserGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"

#include "Editor/Editor.h"

#include "Runtime/Misc/FileSystem.h"
#include "Editor/FileImportMenu/FileImportMenu.h"
#include <GameFramework/GameFramework.h>
#include <GameFramework/Window.h>

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
		if (!ImGui::Begin("ContentBrowser"))
		{
			ImGui::End();
			return;
		}

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
			if (RootFolder)
			{
				DrawFileView();
			}
		}
		ImGui::EndChild();

		ImGui::End();

		//ImGui::ShowDemoWindow();
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

		if ( Node == SelectedFolder)
			tree_flags |= ImGuiTreeNodeFlags_Selected;

		if (!Node->ContainsDirectory())
		tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

		bool node_open = ImGui::TreeNodeEx( "", tree_flags, "%s", Node->File.m_ShortPath.c_str());

		if (ImGui::IsItemFocused() && SelectedFolder != Node)
		{
			SelectedFolder = Node;
			SelectedFile = nullptr;
			
			if (SelectedFolder)
			{
				SelectedFolderFiles = SelectedFolder->GetFiles();
			}
			else
			{
				SelectedFolderFiles.clear();
			}
		}

		if ( node_open )
		{
			for (SystemFileNode* child : Node->Childs)
			{
				if (child && child->File.m_IsDirectory)
				{
					DrawNextFolder( child );
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void ContentBrowserGuiLayer::DrawFileView()
	{
		if ( ImGui::BeginTable( "FileView_1", 1, ImGuiTableFlags_RowBg ) )
		{
			for ( SystemFileNode* File: SelectedFolderFiles )
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::PushID( File->File.m_FullPath.c_str() );

				ImGuiTreeNodeFlags tree_flags = 0;

				if ( File == SelectedFile )
					tree_flags |= ImGuiTreeNodeFlags_Selected;

				bool node_open = ImGui::TreeNodeEx( "", tree_flags, "%s", File->File.m_ShortPath.c_str() );

				if ( ImGui::IsItemFocused() && SelectedFile != File )
				{
					SelectedFile = File;
					Editor::Get()->OnSelectedFile(SelectedFile->File.m_FullPath);
				}

				if ( node_open )
				{
					ImGui::TreePop();
				}

				ImGui::PopID();
			}
		
			ImGui::EndTable();
		}
	}

	void ContentBrowserGuiLayer::OnImport()
	{
		LOG(LogContentBrowser, Info, "Import");

		Editor::Get()->OpenImportMenu("Select file to import", FileImportMenu::FileFilter_Any(), std::bind(&ContentBrowserGuiLayer::OnSelectedFileToImport, this, std::placeholders::_1));
	}

	void ContentBrowserGuiLayer::OnRefresh()
	{
		LOG(LogContentBrowser, Info, "Refresh");
		
		FileSystem::GetFilesInDirectory(Path::GetContentPath(), RootFolder);

		if (!RootFolder)
		{
			LOG(LogContentBrowser, Error, "failed to load content folder.");
		}

		SelectedFolder = RootFolder.get();
		SelectedFile = nullptr;
		SelectedFolderFiles.clear();
	}

	void ContentBrowserGuiLayer::OnSelectedFileToImport( std::string FilePath )
	{
		LOG( LogContentBrowser, Info, "selected file to import.\n\t%s ", FilePath.c_str());

		if (!SelectedFolder)
		{
			LOG( LogContentBrowser, Error, "there is no folder selected in content browser for imprting file. ");
			return;
		}
		
		AssetManager::Get()->Create( FilePath, SelectedFolder->File.m_FullPath );
	}

}

#endif