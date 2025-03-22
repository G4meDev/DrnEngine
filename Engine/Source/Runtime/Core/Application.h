#pragma once

#include <GameFramework/GameFramework.h>

class UpdateEventArgs;
class KeyEventArgs;
class ResizeEventArgs;
class WindowCloseEventArgs;

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

		HINSTANCE m_hInstance;

		std::shared_ptr<Window> m_MainWindow;

		Logger logger;
	};
}