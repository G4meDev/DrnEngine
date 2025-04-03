#include "DrnPCH.h"
#include "ContentBrowserGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#include "Runtime/Misc/FileSystem.h"
#include "Editor/FileImportMenu/FileImportMenu.h"
#include <GameFramework/GameFramework.h>
#include <GameFramework/Window.h>

LOG_DEFINE_CATEGORY( LogContentBrowser, "ContentBrowser" );

namespace Drn
{
	ContentBrowserGuiLayer::ContentBrowserGuiLayer()
	{
		EngineRootFolder = nullptr;
		GameRootFolder = nullptr;

		SelectedFolder = nullptr;
		SelectedFile = nullptr;

		OnRefresh();
	}
	
	ContentBrowserGuiLayer::~ContentBrowserGuiLayer()
	{
		
	}

	void ContentBrowserGuiLayer::Draw( float DeltaTime )
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

			if (EngineRootFolder)
			{
				if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					DrawNextFolder(EngineRootFolder.get());
					ImGui::EndTable();
				}

				if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					DrawNextFolder(GameRootFolder.get());
					ImGui::EndTable();
				}
			}

			ImGui::EndGroup();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("FileView", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if (SelectedFolder)
			{
				DrawFileView();
			}
		}
		ImGui::EndChild();

		ImGui::End();
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
		for ( SystemFileNode* File: SelectedFolderFiles )
		{
			std::string FileName = Path::RemoveFileExtension(File->File.m_ShortPath);
			if ( ImGui::Selectable(FileName.c_str(), SelectedFile == File) )
			{
				SelectedFile = File;
				Editor::Get()->OnSelectedFile( SelectedFile->File.m_FullPath );
			}

			if (ImGui::BeginDragDropSource())
			{
				if ( ImGui::GetDragDropPayload() == NULL )
				{
					ImGui::SetDragDropPayload( EditorConfig::Payload_AssetPath(), File->File.m_FullPath.c_str(), File->File.m_FullPath.size() + 1);
				}

				const ImGuiPayload* Payload = ImGui::GetDragDropPayload();
				auto PayloadAssets = static_cast<const char *>(Payload->Data);

				ImGui::Text( "1 asset selected:\n \t%s", PayloadAssets);

				ImGui::EndDragDropSource();
			}
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
	
		std::string CachedSelectedDirectoryPath = SelectedFolder ? SelectedFolder->File.m_FullPath : "InvalidPath";

		FileSystem::GetFilesInDirectory( Path::GetProjectPath() + "\\Engine\\Content", EngineRootFolder, ".drn" );
		FileSystem::GetFilesInDirectory( Path::GetProjectPath() + "\\Game\\Content", GameRootFolder, ".drn" );

		EngineRootFolder->File.m_ShortPath = "Engine";
		GameRootFolder->File.m_ShortPath = "Game";

		ConvertPath(EngineRootFolder.get());
		ConvertPath(GameRootFolder.get());

		if (!EngineRootFolder)
		{
			LOG(LogContentBrowser, Error, "failed to load engine content folder.");
		}

		if (!GameRootFolder)
		{
			LOG(LogContentBrowser, Error, "failed to load game content folder.");
		}

		SelectedFolder = nullptr;

		GetDirectoryWithPath( EngineRootFolder.get(), CachedSelectedDirectoryPath, &SelectedFolder);
		if (!SelectedFolder)
		{
			GetDirectoryWithPath( GameRootFolder.get(), CachedSelectedDirectoryPath, &SelectedFolder);
		}
		
		if (SelectedFolder)
		{
			SelectedFolderFiles = SelectedFolder->GetFiles();
		}

		else
		{
			SelectedFolderFiles.clear();
		}
		
		SelectedFile = nullptr;
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
		OnRefresh();
	}

	void ContentBrowserGuiLayer::ConvertPath( SystemFileNode* Node )
	{
		Node->File.m_FullPath = Node->File.m_FullPath.substr(9, Node->File.m_FullPath.size() - 9);

		for (SystemFileNode* Child : Node->Childs)
		{
			ConvertPath(Child);
		}
	}

	void ContentBrowserGuiLayer::GetDirectoryWithPath(SystemFileNode* Node, const std::string& Path ,SystemFileNode** Result)
	{
		if (Node->File.m_FullPath == Path)
		{
			*Result = Node;
			return;
		}

		for (SystemFileNode* Child : Node->Childs)
		{
			if (Child->File.m_IsDirectory)
			{
				GetDirectoryWithPath(Child, Path, Result);
			}
		}
	}
}

#endif