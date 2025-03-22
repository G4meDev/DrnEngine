#pragma once

#include "ForwardTypes.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

LOG_DECLARE_CATEGORY(LogApplication);

class Window;

namespace Drn
{
	class Application
	{
	public:
		virtual int Run(HINSTANCE inhInstance);

	protected:
		//virtual void Startup();
		//virtual void Shutdown();
		//
		//virtual void Tick(float DeltaTime);

		void OnUpdate( UpdateEventArgs& e );
		void OnKeyPressed( KeyEventArgs& e );
		void OnWindowResized( ResizeEventArgs& e );
		void OnWindowClose( WindowCloseEventArgs& e );

		std::shared_ptr<Window> m_MainWindow;
		HINSTANCE m_hInstance;
	};
}