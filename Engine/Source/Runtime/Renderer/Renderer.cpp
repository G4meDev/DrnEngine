#include "DrnPCH.h"
#include "Renderer.h"

#include "Runtime/Core/Window.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include <thread>

LOG_DEFINE_CATEGORY( LogRenderer, "Renderer" );

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Drn
{
	Renderer* Renderer::SingletonInstance = nullptr;

	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
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
#if D3D12_DEBUG_LAYER
		ComPtr<ID3D12Debug> debugInterface;
		D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) );
		debugInterface->EnableDebugLayer();
#endif

		m_Device = std::make_unique<Device>();

		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = { };
		CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		m_Device->GetD3D12Device()->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
		
		m_SwapChain = std::make_unique<SwapChain>(m_Device.get(), m_MainWindow->GetWindowHandle(), m_CommandQueue.Get(), m_MainWindow->GetWindowSize());

		for (int i = 0; i < NUM_BACKBUFFERS; i++)
		{
			GetD3D12Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator[i].GetAddressOf()));
		}
		GetD3D12Device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator[m_SwapChain->GetBackBufferIndex()].Get(), NULL, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
		m_CommandList->Close();

		GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
		m_FenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

//#if WITH_EDITOR
//		m_MainScene = AllocateScene(WorldManager::Get()->GetMainWorld());
//#else
//		CameraActor* Cam = WorldManager::Get()->GetMainWorld()->SpawnActor<CameraActor>();
//		Cam->SetActorLocation(XMVectorSet(0, 0, -10, 0));
//
//		m_MainScene = AllocateScene(WorldManager::Get()->GetMainWorld());
//
//		m_MainSceneRenderer = m_MainScene->AllocateSceneRenderer();
//		m_MainSceneRenderer->m_CameraActor = Cam;
//#endif


#if WITH_EDITOR
		ImGuiRenderer::Get()->Init(m_MainWindow);
#endif

		auto commandAllocator = m_CommandAllocator[m_SwapChain->GetBackBufferIndex()];
		commandAllocator->Reset();
		m_CommandList->Reset(commandAllocator.Get(), nullptr);

		//Flush();
	}


	uint64_t Renderer::Signal( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue )
	{
		SCOPE_STAT(RendererSignal);

		uint64_t fenceValueForSignal = ++fenceValue;
		commandQueue->Signal( fence.Get(), fenceValueForSignal );

		return fenceValueForSignal;
	}

	void Renderer::WaitForFenceValue( Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent )
	{
		SCOPE_STAT(RendererWait);

		if (fence->GetCompletedValue() < fenceValue)
		{
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, DWORD_MAX);
		}
	}

	void Renderer::Flush( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent )
	{
		SCOPE_STAT( RendererFlush );

		//uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
		//WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);

		Renderer::Get()->GetCommandList()->Close();
		ID3D12CommandList* const CommandLists[1] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(1, CommandLists);
		uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
		WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);

		m_CommandList->Reset( m_CommandAllocator[m_SwapChain->GetBackBufferIndex()].Get(), NULL );
	}

	void Renderer::Flush()
	{
		Flush(m_CommandQueue, m_Fence, m_FenceValue, m_FenceEvent);
	}

	void Renderer::ReportLiveObjects()
	{
		IDXGIDebug1* dxgiDebug;
		DXGIGetDebugInterface1( 0, IID_PPV_ARGS( &dxgiDebug ) );

		dxgiDebug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL );
		dxgiDebug->Release();
	}

	void Renderer::Shutdown()
	{
		LOG(LogRenderer, Info, "Renderer shutdown.");
		//SingletonInstance->Flush();

#if WITH_EDITOR
		ImGuiRenderer::Get()->Shutdown();
#endif

		for (Scene* S : SingletonInstance->m_AllocatedScenes)
		{
			S->Release();
		}

		CloseHandle(SingletonInstance->m_FenceEvent);

		delete SingletonInstance;
		SingletonInstance = nullptr;
	}

	void Renderer::MainWindowResized( const IntPoint& NewSize ) 
	{
		Flush();
		m_SwapChain->Resize(NewSize);

#ifndef WITH_EDITOR
		m_MainSceneRenderer->ResizeView(NewSize);
#endif
	}

	void Renderer::Tick( float DeltaTime )
	{
		SCOPE_STAT(RendererTick);

		// @TODO: move time to accessible location
		TotalTime += DeltaTime;

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		auto backBuffer = m_SwapChain->GetBackBuffer();

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );

		m_CommandList->ResourceBarrier( 1, &barrier );

		D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_SwapChain->GetBackBufferHandle();
		m_CommandList->ClearRenderTargetView( rtv, clearColor, 0, nullptr );

		for (Scene* S : m_AllocatedScenes)
		{
			S->Render(m_CommandList.Get());
		}

#if WITH_EDITOR
		m_CommandList->OMSetRenderTargets(1, &rtv, false, NULL);
		ImGuiRenderer::Get()->Tick( 1, rtv, m_CommandList.Get() );
#else

		barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_MainSceneRenderer->m_ColorTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE );
		m_CommandList->ResourceBarrier( 1, &barrier );
		barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST );
		m_CommandList->ResourceBarrier( 1, &barrier );

		m_CommandList->CopyResource(backBuffer, m_MainSceneRenderer->m_ColorTarget.Get());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_MainSceneRenderer->m_ColorTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->ResourceBarrier( 1, &barrier );
		barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->ResourceBarrier( 1, &barrier );

#endif

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
		m_CommandList->ResourceBarrier( 1, &barrier );

		{
			SCOPE_STAT( RendererExecuteCommandList );
			m_CommandList->Close();
			ID3D12CommandList* const commandLists[] = { m_CommandList.Get() };
			m_CommandQueue->ExecuteCommandLists( 1, commandLists );
		}

#if WITH_EDITOR
		ImGuiRenderer::Get()->PostExecuteCommands();
#endif

		m_SwapChain->Present();

		WaitForFenceValue( m_Fence, m_SwapChain->m_FrameFenceValues[m_SwapChain->m_CurrentBackbufferIndex], m_FenceEvent );

		auto commandAllocator = m_CommandAllocator[m_SwapChain->GetBackBufferIndex()];
		commandAllocator->Reset();
		m_CommandList->Reset(commandAllocator.Get(), nullptr);
	}

	Scene* Renderer::AllocateScene( World* InWorld )
	{
		Scene* NewScene = new Scene(InWorld);
		m_AllocatedScenes.insert(NewScene);

		return NewScene;
	}

	void Renderer::ReleaseScene( Scene*& InScene )
	{
		m_AllocatedScenes.erase(InScene);

		InScene->Release();
		InScene = nullptr;
	}

}