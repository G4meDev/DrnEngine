#include "DrnPCH.h"
#include "Editor.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include "Editor/Viewport/Viewport.h"
#include "Editor/OutputLog/OutputLog.h"
#include "Editor/ContentBrowser/ContentBrowser.h"
#include "Editor/AssetPreview/AssetPreview.h"

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
		for (int i = 0; i < AssetPreviews.size(); i++)
		{
			std::unique_ptr<AssetPreview>& AssetView = AssetPreviews[i];

			if (AssetView->GetPath() == Path)
			{
				AssetView->SetCurrentFocus();
				return;
			}
		}

		AssetPreviews.push_back(std::unique_ptr<AssetPreview>(AssetPreview::Create(Path)));
	}


}

#endif