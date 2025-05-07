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

		void OnWindowResized( const IntPoint& NewSize );
		void OnKeyPressed( WPARAM Key );

		Window* m_MainWindow;
		HINSTANCE m_hInstance;

		double m_ApplicationTime;

	private:
		void AllocateCons();
	};
}