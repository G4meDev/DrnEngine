#pragma once

#include "DrnPCH.h"

namespace Drn
{
	class Window;

	class Application
	{
	public:
		virtual void Run(HINSTANCE inhInstance);

	protected:
		virtual void Startup();
		virtual void Shutdown();

		virtual void Tick(float DeltaTime);

		bool bRunning = true;

		std::unique_ptr<Window> m_Window = nullptr;
	};
}