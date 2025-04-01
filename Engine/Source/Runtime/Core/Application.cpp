#include "DrnPCH.h"

#include "Application.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include <GameFramework/Events.h>
#include <GameFramework/GameFramework.h>
#include <GameFramework/Window.h>

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

		int retCode = 0;
		auto& gf = GameFramework::Create( m_hInstance );
		{
			WCHAR   path[MAX_PATH];
			HMODULE hModule = ::GetModuleHandleW( NULL );
			if ( ::GetModuleFileNameW( hModule, path, MAX_PATH ) > 0 )
			{
				::PathRemoveFileSpecW( path );
				::SetCurrentDirectoryW( path );
			}

			m_MainWindow = gf.CreateWindow( L"Clear Screen", 1920, 1080 );
			
			m_MainWindow->KeyPressed += KeyboardEvent::slot( &Application::OnKeyPressed, this );
			m_MainWindow->Resize += ResizeEvent::slot( &Application::OnWindowResized, this );
			m_MainWindow->Update += UpdateEvent::slot( &Application::OnUpdate, this );
			m_MainWindow->Close += WindowCloseEvent::slot( &Application::OnWindowClose, this );

			AssetManager::Get()->Init();

			WorldManager::Get()->Init();
			Renderer::Init( inhInstance, m_MainWindow.get() );

#if WITH_EDITOR
			Editor::Get()->Init();
#endif

			m_MainWindow->Show();
		}

		{
			retCode = GameFramework::Get().Run();

#if WITH_EDITOR
			Editor::Get()->Shutdown();
#endif
			
			
			Renderer::Shutdown();
			WorldManager::Shutdown();

			AssetManager::Shutdown();

			m_MainWindow.reset();
		}

		GameFramework::Destroy();

		atexit( &dx12lib::Device::ReportLiveObjects );
		return retCode;
	}


	void Application::OnUpdate( UpdateEventArgs& e ) 
	{
		static uint64_t frameCount = 0;
		static double   totalTime  = 0.0;

		totalTime += e.DeltaTime;
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

		WorldManager::Get()->Tick(e.DeltaTime);

		Renderer::Get()->Tick(e.DeltaTime);

#if WITH_EDITOR
		Editor::Get()->Tick(e.DeltaTime);
#endif
	}

	void Application::OnKeyPressed( KeyEventArgs& e )
	{
		LOG( LogApplication, Info, "KeyPressed: %c", e.Char);

		switch ( e.Key )
		{
		case KeyCode::V:
				if (Renderer::Get())
				{
					Renderer::Get()->ToggleSwapChain();
				}
				break;
		case KeyCode::Escape:
				// Stop the application if the Escape key is pressed.
				GameFramework::Get().Stop();
				break;
		case KeyCode::Enter:
				if ( e.Alt )
				{
					[[fallthrough]];
				case KeyCode::F11:
					m_MainWindow->ToggleFullscreen();
					break;
				}
		}
	}

	void Application::OnWindowResized( ResizeEventArgs& e )
	{
		LOG( LogApplication, Info, "Window Resized: %ix%i", e.Width, e.Height);

		GameFramework::Get().SetDisplaySize( e.Width, e.Height );
		Renderer::Get()->MainWindowResized(e.Width, e.Height);
	}

	void Application::OnWindowClose( WindowCloseEventArgs& e ) 
	{
		GameFramework::Get().Stop();
	}
}