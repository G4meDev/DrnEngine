#include "DrnPCH.h"
#include "Editor.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include "Editor/OutputLog/OutputLog.h"
#include "Editor/ContentBrowser/ContentBrowser.h"
#include "Editor/LevelViewport/LevelViewport.h"
#include "Editor/Misc/TaskGraphVisualizer.h"

#include "Editor/FileImportMenu/FileImportMenu.h"

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
	}

	void Editor::Tick(float DeltaTime)
	{
		SCOPE_STAT();

		OutputLog::Get()->Tick(DeltaTime);
		ContentBrowser::Get()->Tick(DeltaTime);
		//LevelViewport::Get()->Tick(DeltaTime);
	}

	void Editor::Shutdown() 
	{
		CloseImportMenu();

		LevelViewport::Get()->Shutdown();
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
		if (!m_TaskGraphVisualizer)
		{
			m_TaskGraphVisualizer = new TaskGraphVisualizer();
			m_TaskGraphVisualizer->OnLayerClose.BindLambda([](){ Editor::Get()->m_TaskGraphVisualizer = nullptr; });
			m_TaskGraphVisualizer->Attach();
		}
	}
}

#endif