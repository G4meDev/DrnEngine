#include "DrnPCH.h"

#include "Application.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include <Runtime/Core/Window.h>

#include "Editor/Editor.h"

#include <GameFramework/Events.h>
#include <shlwapi.h>

using namespace DirectX;
using namespace Microsoft::WRL;

LOG_DEFINE_CATEGORY( LogApplication, "Application" );

namespace Drn
{
	int Application::Run(HINSTANCE inhInstance)
	{
		m_hInstance = inhInstance;

#if defined( _DEBUG )
		dx12lib::Device::EnableDebugLayer();
#endif

		Startup();

		while (!m_Closing)
		{
			Tick(1);
		}

		Shutdown();
		return 0;
			
// 		m_MainWindow->KeyPressed += KeyboardEvent::slot( &Application::OnKeyPressed, this );
// 		m_MainWindow->Resize += ResizeEvent::slot( &Application::OnWindowResized, this );
// 		m_MainWindow->Update += UpdateEvent::slot( &Application::OnUpdate, this );
// 		m_MainWindow->Close += WindowCloseEvent::slot( &Application::OnWindowClose, this );
	}

	void Application::OnKeyPressed( KeyEventArgs& e )
	{
		//LOG( LogApplication, Info, "KeyPressed: %c", e.Char);

		//switch ( e.Key )
		//{
		//case KeyCode::D1:
		//	if (Profiler::Get())
		//	{
		//		Profiler::Get()->Profile(EProfileMode::Capture_1);
		//	}
		//	break;
		//
		//case KeyCode::D2:
		//	if (Profiler::Get())
		//	{
		//		Profiler::Get()->Profile(EProfileMode::Capture_10);
		//	}
		//	break;
		//	
		//case KeyCode::D3:
		//	if (Profiler::Get())
		//	{
		//		Profiler::Get()->Profile(EProfileMode::Capture_100);
		//	}
		//	break;
		//
		//case KeyCode::V:
		//	if (Renderer::Get())
		//	{
		//		Renderer::Get()->ToggleSwapChain();
		//	}
		//	break;
		//case KeyCode::Escape:
		//	// Stop the application if the Escape key is pressed.
		//	m_Closing = true;
		//	break;
		//case KeyCode::Enter:
		//	if ( e.Alt )
		//	{
		//		[[fallthrough]];
		//	case KeyCode::F11:
		//		m_MainWindow->ToggleFullscreen();
		//		break;
		//	}
		//}
	}

	void Application::OnWindowResized( ResizeEventArgs& e )
	{
		//LOG( LogApplication, Info, "Window Resized: %ix%i", e.Width, e.Height);
		//
		//GameFramework::Get().SetDisplaySize( e.Width, e.Height );
		//
		//if (Renderer::Get())
		//{
		//	Renderer::Get()->MainWindowResized(e.Width, e.Height);
		//}
	}

	void Application::OnWindowClose( WindowCloseEventArgs& e ) 
	{
		//GameFramework::Get().Stop();
	}

// ------------------------------------------------------------------------------------------

	void Application::Startup()
	{
		WCHAR   path[MAX_PATH];
		HMODULE hModule = GetModuleHandleW( NULL );
		if ( GetModuleFileNameW( hModule, path, MAX_PATH ) > 0 )
		{
			PathRemoveFileSpecW( path );
			SetCurrentDirectoryW( path );
		}

		Window::RegisterDefaultClass(m_hInstance);
		m_MainWindow = new Window(m_hInstance, DEFAULT_WINDOW_CLASS_NAME, L"DefaultWindow", IntPoint(1920, 1080));

		Time::Init();
		Profiler::Init();

		AssetManager::Get()->Init();
		PhysicManager::Init();
		Renderer::Init( m_hInstance, m_MainWindow );

		WorldManager::Get()->Init();

#if WITH_EDITOR
		Editor::Get()->Init();
#endif

		m_MainWindow->Show();
	}

	void Application::Shutdown()
	{
		// Renderer::Get()->GetDevice()->Flush();

	#if WITH_EDITOR
		Editor::Get()->Shutdown();
	#endif

		WorldManager::Shutdown();

		Renderer::Shutdown();
		PhysicManager::Shutdown();
		AssetManager::Shutdown();

		Profiler::Shutdown();

		delete m_MainWindow;
		m_MainWindow = nullptr;

		atexit( &dx12lib::Device::ReportLiveObjects );
	}

	void Application::Tick( float DeltaTime )
	{
		SCOPE_STAT(ApplicationTick);

		static uint64_t frameCount = 0;
		static double   totalTime  = 0.0;

		totalTime += DeltaTime;
		frameCount++;

		if ( totalTime > 1.0 )
		{
			auto fps   = frameCount / totalTime;
			frameCount = 0;
			totalTime -= 1.0;

			LOG( LogApplication, Info, "FPS: %i", (int)fps);

			wchar_t buffer[256];
			::swprintf_s( buffer, L"Cube [FPS: %f]", fps );
			m_MainWindow->SetWindowTitle( buffer );
		}

		Profiler::Get()->Tick(DeltaTime);

		PhysicManager::Get()->Tick(DeltaTime);
		WorldManager::Get()->Tick(DeltaTime);

		Renderer::Get()->Tick(DeltaTime);

#if WITH_EDITOR
		Editor::Get()->Tick(DeltaTime);
#endif
	}


}