#include "DrnPCH.h"
#include "Editor.h"

#if WITH_EDITOR
#include "Editor/Viewport/Viewport.h"
#include "Editor/OutputLog/OutputLog.h"

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
		Viewport::Get()->Init();
		//OutputLog::Get()->Init();
	}

	void Editor::Tick(float DeltaTime)
	{
		Viewport::Get()->Tick(DeltaTime);
		//OutputLog::Get()->Tick(DeltaTime);
	}

	Editor* Editor::Get()
	{
		if (!SingletonInstance.get())
		{
			SingletonInstance = std::make_unique<Editor>();
		}

		return SingletonInstance.get();
	}
}

#endif