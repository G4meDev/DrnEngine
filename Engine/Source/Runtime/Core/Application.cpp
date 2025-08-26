#include "DrnPCH.h"

#include "Application.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include <Runtime/Core/Window.h>

#include "Editor/Editor.h"

#include <shlwapi.h>

#include <taskflow.hpp>

#include <corecrt_io.h>
#include <fcntl.h>
#define MAX_CONSOLE_LINES 500;

using namespace DirectX;
using namespace Microsoft::WRL;

LOG_DEFINE_CATEGORY( LogApplication, "Application" );

namespace Drn
{
	tf::Taskflow Application::taskflow;

	int Application::Run( HINSTANCE inhInstance )
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
		case VK_F3:
#if WITH_EDITOR
			if (Editor::Get())
			{
				Editor::Get()->OpenTaskGraphVisualizer();
			}
			break;
#endif

		case VK_F4:
			if (WorldManager::Get())
			{
				WorldManager::Get()->GetMainWorld()->GetPhysicScene()->ToggleShowCollision();
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

		InputManager::Get()->Init();
		AssetManager::Get()->Init();
		PhysicManager::Init();
		Renderer::Init( m_hInstance, m_MainWindow );

		WorldManager::Get()->Init();

#if WITH_EDITOR
		Editor::Get()->Init();
#endif

		//ID3D12CommandQueue* C[] = { Renderer::Get()->m_CommandQueue.Get() };
		//OPTICK_GPU_INIT_D3D12(Renderer::Get()->GetD3D12Device(), C, 1);

		m_MainWindow->OnWindowResize.Add(this, &Application::OnWindowResized);
		m_MainWindow->OnKeyPress.Add(this, &Application::OnKeyPressed);
		m_MainWindow->Show();

		auto WorldTick = taskflow.emplace( [&]() {OPTICK_THREAD_TASK(); WorldManager::Get()->Tick(m_DeltaTime); } );
		auto PhysicTick = taskflow.emplace( [&]() {OPTICK_THREAD_TASK(); PhysicManager::Get()->Tick(m_DeltaTime); } );
		
		tf::Task RendererTick = taskflow.composed_of(Renderer::Get()->m_RendererTickTask);

		WorldTick.precede(PhysicTick);
		WorldTick.precede(RendererTick);

#if WITH_EDITOR
		auto EditorTick = taskflow.emplace( [&]() {OPTICK_THREAD_TASK(); Editor::Get()->Tick(m_DeltaTime); } );
		RendererTick.precede(EditorTick);

		WorldTick.name("WorldTick");
		PhysicTick.name("PhysicTick");
		RendererTick.name("RendererTick");
		EditorTick.name("EditorTick");
#endif
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

		InputManager::Get()->Shutdown();

		m_MainWindow->OnWindowResize.Remove(this);
		m_MainWindow->OnKeyPress.Remove(this);

		delete m_MainWindow;
		m_MainWindow = nullptr;

		std::atexit(Renderer::ReportLiveObjects);
	}

	void Application::Tick( float DeltaTime )
	{
		OPTICK_FRAME( "MainThread" );
		SCOPE_STAT();

		m_DeltaTime = DeltaTime;
		Time::SetApplicationDeltaTime(m_DeltaTime);

		HandleWindowMessages();
		UpdateWindowTitle(DeltaTime);

		InputManager::Get()->Tick(m_DeltaTime);

#if 0

		WorldManager::Get()->Tick(m_DeltaTime);
		PhysicManager::Get()->Tick(m_DeltaTime);
		Renderer::Get()->Tick(m_DeltaTime);
		Editor::Get()->Tick(m_DeltaTime);

#else
		Taskflow::GetExecuter().run(taskflow).wait();
#endif

#if WITH_EDITOR
		// this should run in main thread.
		ImGuiRenderer::Get()->PostExecuteCommands();
#endif
	}

	void Application::HandleWindowMessages() const
	{
		SCOPE_STAT();

		MSG msg;
		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			InputManager::Get()->Get()->HandleMessage(msg);
		}
	}

	void Application::UpdateWindowTitle( float DeltaTime ) const
	{
		SCOPE_STAT();

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
			::swprintf_s( buffer, L"[FPS: %f]", fps );
			m_MainWindow->SetWindowTitle( buffer );
		}
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