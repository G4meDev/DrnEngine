#include "DrnPCH.h"
#include "Application.h"
#include "Window.h"
#include "Runtime/Renderer/Renderer.h"

#include "Editor/Editor.h"

namespace Drn
{
	void Application::Run(HINSTANCE inhInstance)
	{
		std::cout << "Start application" << std::endl;

		Renderer::Init(inhInstance);
		
#if WITH_EDITOR
		Editor::Get()->Init();
#endif

		Startup();

		while (bRunning && !Renderer::Get()->GetMainWindow()->PendingClose())
		{
			Tick(1.0f);
		}

		Shutdown();
	}

	void Application::Startup()
	{

	}

	void Application::Shutdown()
	{
		std::cout << "Shutdown application" << std::endl;

		Renderer::Get()->Shutdown();
	}

	void Application::Tick(float DeltaTime)
	{
		Renderer::Get()->Tick(DeltaTime);

#if WITH_EDITOR
		Editor::Get()->Tick(DeltaTime);
#endif

		Renderer::Get()->WaitForPreviousFrame();
	}
}