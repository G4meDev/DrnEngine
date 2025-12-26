#pragma once

#include "ForwardTypes.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#include <taskflow.hpp>

LOG_DECLARE_CATEGORY(LogApplication);

namespace Drn
{
	class Window;

	class Application
	{
	public:
		virtual int Run(HINSTANCE inhInstance);

		static tf::Taskflow taskflow;

	protected:
		virtual void Startup();
		virtual void Shutdown();
		
		virtual void Tick(float DeltaTime);

		void OnWindowResized( const IntPoint& NewSize );
		void OnKeyPressed( WPARAM Key , LPARAM lParam);

		Window* m_MainWindow;
		HINSTANCE m_hInstance;

		double m_ApplicationTime;

	private:
		void AllocateCons();

		void HandleWindowMessages() const;
		void UpdateWindowTitle(float DeltaTime) const;

		float m_DeltaTime;

	};
}