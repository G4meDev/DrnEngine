#include "DrnPCH.h"
#include "Renderer.h"

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include <GameFramework/Window.h>
#include <GameFramework/GameFramework.h>


LOG_DEFINE_CATEGORY( LogRenderer, "Renderer" );

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Drn
{
	Renderer* Renderer::SingletonInstance;

	Renderer::Renderer() 
	{
		
	}

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init( HINSTANCE inhInstance, Window* InMainWindow )
	{
		SingletonInstance = new Renderer();

		SingletonInstance->m_MainWindow = InMainWindow;
		SingletonInstance->Init_Internal();
	}

	void Renderer::Init_Internal() 
	{
		m_Device = dx12lib::Device::Create();

		auto        description = m_Device->GetDescription();
		std::string description_str( StringHelper::ws2s(description) );
		LOG( LogRenderer, Info, "%s", description_str.c_str() );

		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
		m_CommandList = commandQueue.GetCommandList();

// -------------------------------------------------------------------------------

		std::string full = Path::ConvertFullPath( "BasicShapes\\SM_Cube.drn" );
		AssetHandle<StaticMesh> CubeMesh(Path::ConvertFullPath("BasicShapes\\SM_Cube.drn"));
		CubeMesh.Load();

		MainWorld = WorldManager::Get()->AllocateWorld();

		m_CubeStaticMeshActor = MainWorld->SpawnActor<StaticMeshActor>();
		m_CubeStaticMeshActor->GetMeshComponent()->SetMesh(CubeMesh);

		m_CameraActor = MainWorld->SpawnActor<CameraActor>();
		m_CameraActor->SetActorLocation(XMVectorSet(0, 0, -10, 0));

		MainScene = Renderer::Get()->AllocateScene( MainWorld );
		MainSceneRenderer = MainScene->AllocateSceneRenderer();

		MainSceneRenderer->m_CameraActor = m_CameraActor;

// -------------------------------------------------------------------------------

		m_SwapChain = m_Device->CreateSwapChain( m_MainWindow->GetWindowHandle(), DXGI_FORMAT_R8G8B8A8_UNORM );
		m_SwapChain->SetVSync( false );

		commandQueue.Flush();

#if WITH_EDITOR
		ImGuiRenderer::Get()->Init();
#endif
	}

	void Renderer::Shutdown()
	{
		LOG(LogRenderer, Info, "Renderer shutdown.");

		if ( SingletonInstance->MainScene)
		{
			if ( SingletonInstance->MainSceneRenderer)
			{
				SingletonInstance->MainScene->RemoveAndInvalidateSceneRenderer( SingletonInstance->MainSceneRenderer);
			}

			Renderer::Get()->RemoveAndInvalidateScene( SingletonInstance->MainScene);
		}

		if ( SingletonInstance->MainWorld)
		{
			WorldManager::Get()->RemoveAndInvalidateWorld(SingletonInstance->MainWorld);
		}


#if WITH_EDITOR
		ImGuiRenderer::Get()->Shutdown();
#endif

		SingletonInstance->m_CommandList.reset();
		SingletonInstance->m_SwapChain.reset();
		SingletonInstance->m_Device.reset();

		delete SingletonInstance;
		SingletonInstance = nullptr;
	}

	void Renderer::ToggleSwapChain() 
	{
		m_SwapChain->ToggleVSync();
	}

	void Renderer::MainWindowResized( float InWidth, float InHeight ) 
	{
		m_Device->Flush();
		m_SwapChain->Resize( InWidth, InHeight );

#ifndef WITH_EDITOR
		MainSceneRenderer->ResizeView(IntPoint(InWidth, InHeight));
#endif
	}

	void Renderer::Tick( float DeltaTime )
	{
		// @TODO: move time to accessible location
		TotalTime += DeltaTime;

		XMVECTOR Location = XMVectorSet(sin(TotalTime) * 5, 0, 0, 1);
		m_CubeStaticMeshActor->SetActorLocation(Location);

		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
		m_CommandList  = commandQueue.GetCommandList();

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

		auto& swapChainRT         = m_SwapChain->GetRenderTarget();
		auto  swapChainBackBuffer = swapChainRT.GetTexture( dx12lib::AttachmentPoint::Color0 );
		auto  msaaRenderTarget    = MainSceneRenderer->m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 );

		for (Scene* S : AllocatedScenes)
		{
			S->Render(m_CommandList.get());
		}

		m_CommandList->SetRenderTarget( swapChainRT );
		m_CommandList->ClearTexture( swapChainRT.GetTexture( dx12lib::AttachmentPoint::Color0 ), clearColor );

#if WITH_EDITOR
		ImGuiRenderer::Get()->Tick( 1, swapChainBackBuffer->GetRenderTargetView(),
									m_CommandList->GetD3D12CommandList().Get() );
#else
		// m_CommandList->ResolveSubresource( swapChainBackBuffer, msaaRenderTarget );
		m_CommandList->CopyResource( swapChainBackBuffer, msaaRenderTarget );

#endif

		commandQueue.ExecuteCommandList( m_CommandList );

#if WITH_EDITOR
		ImGuiRenderer::Get()->PostExecuteCommands();
#endif

		m_SwapChain->Present();
	}

	Scene* Renderer::AllocateScene( World* InWorld )
	{
		Scene* NewScene = new Scene(InWorld);
		AllocatedScenes.insert(NewScene);

		return NewScene;
	}

	void Renderer::RemoveScene( Scene* InScene )
	{
		AllocatedScenes.erase(InScene);
	}

	void Renderer::RemoveAndInvalidateScene( Scene*& InScene )
	{
		RemoveScene(InScene);
		delete InScene;
		InScene = nullptr;
	}

}