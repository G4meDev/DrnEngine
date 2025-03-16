#include "DrnPCH.h"
#include "Application.h"
#include "Window.h"
#include "Runtime/Renderer/Renderer.h"

namespace Drn
{
	void Application::Run(HINSTANCE inhInstance)
	{
		std::cout << "Start application" << std::endl;

		Renderer::Init(inhInstance);
		

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
	}
}