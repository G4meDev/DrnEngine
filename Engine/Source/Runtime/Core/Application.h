#pragma once

#include "ForwardTypes.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

LOG_DECLARE_CATEGORY(LogApplication);


namespace Drn
{
	class Window;

	class Application
	{
	public:
		virtual int Run(HINSTANCE inhInstance);

	protected:
		virtual void Startup();
		virtual void Shutdown();
		
		virtual void Tick(float DeltaTime);


		void OnKeyPressed( KeyEventArgs& e );
		void OnWindowResized( ResizeEventArgs& e );
		void OnWindowClose( WindowCloseEventArgs& e );

		Window* m_MainWindow;
		HINSTANCE m_hInstance;

		bool m_Closing = false;
	};
}