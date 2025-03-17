#include "DrnPCH.h"
#include "Editor.h"

#if WITH_EDITOR
#include "Editor/Viewport/Viewport.h"

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
	}

	void Editor::Tick(float DeltaTime)
	{
		Viewport::Get()->Tick(DeltaTime);
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