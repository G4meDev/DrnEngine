#include "DrnPCH.h"

#include "Application.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include <Runtime/Core/Window.h>

#include "Editor/Editor.h"

#include <shlwapi.h>

#include <corecrt_io.h>
#include <fcntl.h>
#define MAX_CONSOLE_LINES 500;

using namespace DirectX;
using namespace Microsoft::WRL;

LOG_DEFINE_CATEGORY( LogApplication, "Application" );

namespace Drn
{
	int Application::Run(HINSTANCE inhInstance)
	{
		m_hInstance = inhInstance;

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
		
		case VK_SPACE:
			if (Renderer::Get())
			{
				Renderer::Get()->ToggleVSync();
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

		AllocateCons();

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
		Renderer::Get()->Flush();

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

		std::atexit(Renderer::ReportLiveObjects);
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

	void Application::AllocateCons()
	{
		if ( AllocConsole() )
		{
			HANDLE lStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );

			CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
			GetConsoleScreenBufferInfo( lStdHandle, &consoleInfo );
			consoleInfo.dwSize.Y = MAX_CONSOLE_LINES;
			SetConsoleScreenBufferSize( lStdHandle, consoleInfo.dwSize );
			SetConsoleCursorPosition( lStdHandle, { 0, 0 } );

			int   hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
			FILE* fp         = _fdopen( hConHandle, "w" );
			freopen_s( &fp, "CONOUT$", "w", stdout );
			setvbuf( stdout, nullptr, _IONBF, 0 );

			lStdHandle = GetStdHandle( STD_INPUT_HANDLE );
			hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
			fp         = _fdopen( hConHandle, "r" );
			freopen_s( &fp, "CONIN$", "r", stdin );
			setvbuf( stdin, nullptr, _IONBF, 0 );

			lStdHandle = GetStdHandle( STD_ERROR_HANDLE );
			hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
			fp         = _fdopen( hConHandle, "w" );
			freopen_s( &fp, "CONOUT$", "w", stderr );
			setvbuf( stderr, nullptr, _IONBF, 0 );

			std::wcout.clear();
			std::cout.clear();
			std::wcerr.clear();
			std::cerr.clear();
			std::wcin.clear();
			std::cin.clear();
		}
	}

}