#include "DrnPCH.h"
#include "Editor.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include "Editor/OutputLog/OutputLog.h"
#include "Editor/ContentBrowser/ContentBrowser.h"
#include "Editor/StatsWindow/StatsWindow.h"
#include "Editor/LevelViewport/LevelViewport.h"
#include "Editor/Misc/TaskGraphVisualizer.h"

#include "Editor/Misc/EditorMisc.h"
#include "Editor/Thumbnail/ThumbnailManager.h"

#include "Editor/FileImportMenu/FileImportMenu.h"

#include "Runtime/Core/Application.h"

LOG_DEFINE_CATEGORY(LogEditor, "Editor");

namespace Drn
{
	std::unique_ptr<Editor> Editor::SingletonInstance;

	Editor::Editor()
		: m_TaskGraphVisualizer(nullptr)
	{
		if (WorldManager::Get())
		{
			WorldManager::Get()->OnOpenLevel.Add( this, &Editor::OnOpenLevel);
			WorldManager::Get()->OnCloseLevel.Add( this, &Editor::OnCloseLevel);
		}
	}

	Editor::~Editor()
	{
		if (WorldManager::Get())
		{
			WorldManager::Get()->OnOpenLevel.Remove( this);
			WorldManager::Get()->OnCloseLevel.Remove( this);
		}
	}

	void Editor::Init()
	{
		OutputLog::Get()->Init();
		ContentBrowser::Get()->Init();
		LevelViewport::Get()->Init( WorldManager::Get()->GetMainWorld() );
		StatsWindow::Get()->Init();

		EditorMisc::Get()->Register();
		ThumbnailManager::Get()->Init();
	}

	void Editor::Tick(float DeltaTime)
	{
		SCOPE_STAT();

		OutputLog::Get()->Tick(DeltaTime);
		ContentBrowser::Get()->Tick(DeltaTime);
		StatsWindow::Get()->Tick(DeltaTime);
		LevelViewport::Get()->Tick(DeltaTime);

		ThumbnailManager::Get()->Tick(DeltaTime);
	}

	void Editor::Shutdown() 
	{
		CloseImportMenu();

		ThumbnailManager::Get()->Shutdown();

		LevelViewport::Get()->Shutdown();
		StatsWindow::Get()->Shutdown();
		ContentBrowser::Get()->Shutdown();
		OutputLog::Get()->Shutdown();
	}

	Editor* Editor::Get()
	{
		if (!SingletonInstance.get())
		{
			SingletonInstance = std::make_unique<Editor>();
		}

		return SingletonInstance.get();
	}

	void Editor::OnSelectedFile( const std::string Path )
	{
		AssetHandle<Asset> SelectedAsset(Path);
		SelectedAsset.LoadGeneric();
		SelectedAsset.Get()->OpenAssetPreview();
	}

	std::shared_ptr<Drn::FileImportMenu> Editor::OpenImportMenu(const std::string& InDisplayText, const char* InFilters, std::function<void(std::string)> InOnSelectedFile)
	{
		CloseImportMenu();

		m_FileImportMenu = std::make_shared<FileImportMenu>(InDisplayText, InFilters, InOnSelectedFile);
		return m_FileImportMenu;
	}

	void Editor::CloseImportMenu()
	{
		m_FileImportMenu.reset();
	}

	void Editor::NotifyMaterialReimported( const AssetHandle<Material>& Mat )
	{
		return;

		for (World* W : WorldManager::Get()->m_AllocatedWorlds)
		{
			for (Actor* actor : W->GetActorList())
			{
				std::vector<StaticMeshComponent*> MeshComponents;
				actor->GetRoot()->GetComponents<StaticMeshComponent>(MeshComponents, EComponentType::StaticMeshComponent, true);

				for (StaticMeshComponent* MC : MeshComponents)
				{
					if (MC && MC->IsUsingMaterial(Mat))
					{
						MC->MarkRenderStateDirty();
					}
				}
			}
		}
	}

	void Editor::OnOpenLevel( World* OpenedWorld )
	{
		LevelViewport::Init( OpenedWorld );
	}

	void Editor::OnCloseLevel( World* ClosedWorld )
	{
		LevelViewport::Shutdown();
	}

	void Editor::OpenTaskGraphVisualizer()
	{
		const std::string TaskGraphDump = Application::taskflow.dump();

		DateTime TimeStamp = DateTime::Now();
		std::string FilePath = ".\\Saved\\TaskflowVisualizer\\Tf-" + TimeStamp.ToStringFileStamp();
		std::string FilePathDot = FilePath + ".dot";
		std::string FilePathPng = FilePath + ".png";
		FileSystem::WriteStringToFile( FilePathDot, TaskGraphDump );

		std::string cmd = "..\\..\\..\\Tools\\Graphvis\\dot -Tpng " + FilePathDot + " -o" + FilePathPng;
		system(cmd.c_str());
		ShellExecute(NULL, "open", FilePathPng.c_str(), NULL, NULL, NULL);

		//if (!m_TaskGraphVisualizer)
		//{
		//	m_TaskGraphVisualizer = new TaskGraphVisualizer();
		//	m_TaskGraphVisualizer->OnLayerClose.BindLambda([](){ Editor::Get()->m_TaskGraphVisualizer = nullptr; });
		//	m_TaskGraphVisualizer->Attach();
		//}
	}

	template<typename T>
	void Editor::ReimportAsset()
	{
		std::unique_ptr<SystemFileNode> EngineRootFolder;
		std::unique_ptr<SystemFileNode> GameRootFolder;

		FileSystem::GetFilesInDirectory( Path::GetProjectPath() + "\\Engine\\Content", EngineRootFolder, ".drn" );
		FileSystem::GetFilesInDirectory( Path::GetProjectPath() + "\\Game\\Content", GameRootFolder, ".drn" );

		std::vector<SystemFileNode*> EngineFiles;
		EngineRootFolder->GetFilesRecursive(EngineFiles);

		std::vector<SystemFileNode*> GameFiles;
		GameRootFolder->GetFilesRecursive(GameFiles);

		auto ImportAssetConditional = [](std::string& FilePath)
		{
			std::string AssetPath = FilePath.substr(9, FilePath.size() - 9);

			AssetHandle<T>MatAsset (AssetPath);
			if (MatAsset.ValidateType())
			{
				MatAsset.Load();
				MatAsset->Import();
			}
		};

		for (SystemFileNode* Node : EngineFiles)
		{
			ImportAssetConditional(Node->File.m_FullPath);
		}

		for (SystemFileNode* Node : GameFiles)
		{
			ImportAssetConditional(Node->File.m_FullPath);
		}
	}

	void Editor::ReimportMaterials()
	{
		ReimportAsset<Material>();
	}

	void Editor::ReimportStaticMeshes()
	{
		ReimportAsset<StaticMesh>();
	}

	IntPoint Editor::GetScreenPositionRelative()
	{
		const ImVec2 RectMin = ImGui::GetItemRectMin();
		const ImVec2 RectMax = ImGui::GetItemRectMax();
		const ImVec2 MousePos = ImGui::GetMousePos();
		const IntPoint ScreenPos = IntPoint( MousePos.x - RectMin.x, MousePos.y - RectMin.y );

		return ScreenPos;
	}

}

#endif