#include "DrnPCH.h"
#include "Editor.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include "Editor/Viewport/Viewport.h"
#include "Editor/OutputLog/OutputLog.h"
#include "Editor/ContentBrowser/ContentBrowser.h"
#include "Editor/AssetPreview/AssetPreview.h"

#include "Editor/FileImportMenu/FileImportMenu.h"

LOG_DEFINE_CATEGORY(LogEditor, "Editor");

namespace Drn
{
	std::unique_ptr<Editor> Editor::SingletonInstance;

	Editor::Editor()
	{ }

	Editor::~Editor()
	{ }

	void Editor::Init()
	{
		OutputLog::Get()->Init();
		Viewport::Get()->Init();
		ContentBrowser::Get()->Init();
	}

	void Editor::Tick(float DeltaTime)
	{
		OutputLog::Get()->Tick(DeltaTime);
		Viewport::Get()->Tick(DeltaTime);
		ContentBrowser::Get()->Tick(DeltaTime);
	}

	void Editor::Shutdown() 
	{
		CloseImportMenu();
		CloseAssetPreviews();

		ContentBrowser::Get()->Shutdown();
		Viewport::Get()->Shutdown();
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
		OpenAssetView(Path);
	}

	void Editor::OpenAssetView( const std::string Path )
	{
		for (int i = 0; i < m_AssetPreviews.size(); i++)
		{
			std::shared_ptr<AssetPreview>& AssetView = m_AssetPreviews[i];

			if (AssetView->GetPath() == Path)
			{
				AssetView->SetCurrentFocus();
				return;
			}
		}

		m_AssetPreviews.push_back(AssetPreview::Open(Path));
	}

	void Editor::CloseAssetPreviews()
	{
		m_AssetPreviews.clear();
	}

	std::shared_ptr<Drn::FileImportMenu> Editor::OpenImportMenu(const std::string& InDisplayText, char* InFilters, std::function<void(std::string)> InOnSelectedFile)
	{
		CloseImportMenu();

		m_FileImportMenu = std::make_shared<FileImportMenu>(InDisplayText, InFilters, InOnSelectedFile);
		return m_FileImportMenu;
	}

	void Editor::CloseImportMenu()
	{
		m_FileImportMenu.reset();
	}

}

#endif