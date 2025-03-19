#include "DrnPCH.h"
#include "Application.h"
#include "Runtime/Renderer/Renderer.h"

#include "Editor/Editor.h"

namespace Drn
{
	int Application::Run(HINSTANCE inhInstance)
	{
		m_hInstance = inhInstance;

#if defined( _DEBUG )
		// Always enable the Debug layer before doing anything with DX12.
		dx12lib::Device::EnableDebugLayer();
#endif

		int retCode = 0;
		auto& gf = GameFramework::Create( m_hInstance );
		{
			logger = gf.CreateLogger( "ClearScreen" );

			pDevice = dx12lib::Device::Create();

			auto description = pDevice->GetDescription();
			logger->info( L"Device Created: {}", description );

			pGameWindow = gf.CreateWindow( L"Clear Screen", 1920, 1080 );

			pSwapChain = pDevice->CreateSwapChain( pGameWindow->GetWindowHandle() );
			pSwapChain->SetVSync( false );

			pGameWindow->KeyPressed += KeyboardEvent::slot( &Application::OnKeyPressed, this );
			pGameWindow->Resize += ResizeEvent::slot( &Application::OnWindowResized, this );
			pGameWindow->Update += UpdateEvent::slot( &Application::OnUpdate, this );
			pGameWindow->Close += WindowCloseEvent::slot( &Application::OnWindowClose, this );

			pGameWindow->Show();

			retCode = GameFramework::Get().Run();

			pSwapChain.reset();
			pGameWindow.reset();
			pDevice.reset();
		}

		GameFramework::Destroy();

		atexit(&dx12lib::Device::ReportLiveObjects);
		return retCode;
	}


	void Application::OnUpdate( UpdateEventArgs& e ) 
	{
		static uint64_t frameCount = 0;
		static double   totalTime  = 0.0;

		totalTime += e.DeltaTime;
		++frameCount;

		if ( totalTime > 1.0 )
		{
			auto fps   = frameCount / totalTime;
			frameCount = 0;
			totalTime  = 0.0;

			logger->info( "FPS: {:.7}", fps );

			wchar_t buffer[256];
			::swprintf_s( buffer, L"Clear Screen [FPS: %f]", fps );
			pGameWindow->SetWindowTitle( buffer );
		}

		auto& commandQueue = pDevice->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
		auto  commandList  = commandQueue.GetCommandList();

		auto& renderTarget = pSwapChain->GetRenderTarget();

		const FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		commandList->ClearTexture( renderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 ), clearColor );

		commandQueue.ExecuteCommandList( commandList );

		pSwapChain->Present();
	}

	void Application::OnKeyPressed( KeyEventArgs& e )
	{
		logger->info( L"KeyPressed: {}", (wchar_t)e.Char );

		switch ( e.Key )
		{
		case KeyCode::V:
				pSwapChain->ToggleVSync();
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
					pGameWindow->ToggleFullscreen();
					break;
				}
		}
	}

	void Application::OnWindowResized( ResizeEventArgs& e )
	{
		logger->info( "Window Resize: {}, {}", e.Width, e.Height );
		pSwapChain->Resize( e.Width, e.Height );
	}

	void Application::OnWindowClose( WindowCloseEventArgs& e ) 
	{
		GameFramework::Get().Stop();
	}

// 		Startup();
// 
// 		while (bRunning && !Renderer::Get()->GetMainWindow()->PendingClose())
// 		{
// 			float DeltaTime = m_Timer.ElapsedSeconds();
// 			m_Timer.Tick();
// 
// 			Tick(DeltaTime);
// 		}
// 
// 		Shutdown();
// 	}
// 
// 	void Application::Startup()
// 	{
// 		std::cout << "Start application" << std::endl;
// 
// 		Renderer::Init(m_hInstance);
// 
// #if WITH_EDITOR
// 		Editor::Get()->Init();
// #endif
// 
// 
// 	}
// 
// 	void Application::Shutdown()
// 	{
// 		std::cout << "Shutdown application" << std::endl;
// 
// 		Renderer::Get()->Shutdown();
// 	}
// 
// 	void Application::Tick(float DeltaTime)
// 	{
// 		Renderer::Get()->Tick(DeltaTime);
// 
// #if WITH_EDITOR
// 		Editor::Get()->Tick(DeltaTime);
// #endif
// 	}


}