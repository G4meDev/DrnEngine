#include "DrnPCH.h"

#include "Application.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include <Runtime/Core/Window.h>

#include "Editor/Editor.h"

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

		while (!m_MainWindow->IsClosing())
		{
			double CurrentTime = Time::GetSeconds();
			double DeltaTime = CurrentTime - m_ApplicationTime;
			m_ApplicationTime = CurrentTime;

			Tick(DeltaTime);
		}

		Shutdown();
		return 0;
	}

	void Application::OnKeyPressed( WPARAM Key )
	{
		switch ( Key )
		{
		case VK_F1:
			if (Profiler::Get())
			{
				Profiler::Get()->Profile(EProfileMode::Capture_1);
			}
			break;
		
		case VK_F2:
			if (Profiler::Get())
			{
				Profiler::Get()->Profile(EProfileMode::Capture_10);
			}
			break;
			
		case VK_F3:
			if (Profiler::Get())
			{
				Profiler::Get()->Profile(EProfileMode::Capture_100);
			}
			break;
		
		case VK_F10:
			if (Renderer::Get())
			{
				Renderer::Get()->ToggleSwapChain();
			}
			break;
		case VK_ESCAPE:
			m_MainWindow->SetClosing();
			break;
		case VK_F11:
			m_MainWindow->ToggleFullScreen();
			break;
		}
	}

	void Application::OnWindowResized( const IntPoint& NewSize )
	{
		if (Renderer::Get())
		{
			Renderer::Get()->MainWindowResized(NewSize);
		}
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
		m_ApplicationTime = Time::GetSeconds();

		Profiler::Init();

		AssetManager::Get()->Init();
		PhysicManager::Init();
		Renderer::Init( m_hInstance, m_MainWindow );

		WorldManager::Get()->Init();

#if WITH_EDITOR
		Editor::Get()->Init();
#endif

		m_MainWindow->BindOnSizeChanged( std::bind( &Application::OnWindowResized, this, std::placeholders::_1 ) );
		m_MainWindow->BindOnKeyPress( std::bind( &Application::OnKeyPressed, this, std::placeholders::_1 ) );
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

		MSG msg;
		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

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