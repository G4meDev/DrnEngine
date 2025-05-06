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
		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;
#if WITH_EDITOR
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		bool UseWarp = true;

		if (UseWarp)
		{
			dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));
			dxgiAdapter1.As(&dxgiAdapter4);
		}

		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), 
						D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && 
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory )
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					dxgiAdapter1.As(&dxgiAdapter4);
				}
			}
		}

		D3D12CreateDevice(dxgiAdapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_Device.GetAddressOf()));

#if WITH_EDITOR && 0
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(m_Device.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};
 
			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                      
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                    
			};
 
			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;
 
			ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
		}
#endif

		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = { };
		CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		m_Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
		m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf()));
		m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), NULL, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
		//m_CommandList->Close();

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width                 = m_MainWindow->GetWindowSize().X;
		swapChainDesc.Height                = m_MainWindow->GetWindowSize().Y;
		swapChainDesc.Format                = DISPLAY_OUTPUT_FORMAT;
		swapChainDesc.Stereo                = FALSE;
		swapChainDesc.SampleDesc            = { 1, 0 };
		swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount           = NUM_BACKBUFFERS;
		swapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;

		ComPtr<IDXGISwapChain1> swapChain1;
		dxgiFactory->CreateSwapChainForHwnd( m_CommandQueue.Get(), m_MainWindow->GetWindowHandle(), &swapChainDesc, nullptr, nullptr, &swapChain1 );
		swapChain1.As( &m_SwapChain );
		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = NUM_BACKBUFFERS;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap));
		UpdateRenderTargetViews();

		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
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

		Flush();
	}

	void Renderer::UpdateRenderTargetViews()
	{
		m_RTVDescriporSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
 
		for (int i = 0; i < NUM_BACKBUFFERS; ++i)
		{
			ComPtr<ID3D12Resource> backBuffer;
			m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
 
			m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
			m_BackBuffers[i] = backBuffer;
			rtvHandle.Offset(m_RTVDescriporSize);
		}
	}

	uint64_t Renderer::Signal()
	{
		uint64 fenceValueForSignal = ++m_FenceValue;
		m_CommandQueue->Signal( m_Fence.Get(), fenceValueForSignal );

		return fenceValueForSignal;
	}

	void Renderer::WaitForFenceValue( uint64 Value )
	{
		if ( m_Fence->GetCompletedValue() < Value )
		{
			m_Fence->SetEventOnCompletion( Value, m_FenceEvent );
			WaitForSingleObject( m_FenceEvent, static_cast<DWORD>( std::chrono::milliseconds::max().count() ) );
		}
	}

	void Renderer::Flush()
	{
		Renderer::Get()->GetCommandList()->Close();
		ID3D12CommandList* const CommandLists[1] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(1, CommandLists);

		//std::this_thread::sleep_for(std::chrono::milliseconds(100));

		uint64_t fenceValueForSignal = Signal();
		WaitForFenceValue(fenceValueForSignal);

		//m_CommandAllocator->Reset();
		m_CommandList->Reset( m_CommandAllocator.Get(), NULL );
	}

	void Renderer::Shutdown()
	{
		LOG(LogRenderer, Info, "Renderer shutdown.");


#if WITH_EDITOR
		ImGuiRenderer::Get()->Shutdown();
#endif

		for (Scene* S : SingletonInstance->m_AllocatedScenes)
		{
			delete S;
		}

		SingletonInstance->Flush();
		CloseHandle(SingletonInstance->m_FenceEvent);

		IDXGIDebug1* dxgiDebug;
		DXGIGetDebugInterface1( 0, IID_PPV_ARGS( &dxgiDebug ) );

		dxgiDebug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL );
		dxgiDebug->Release();

		delete SingletonInstance;
		SingletonInstance = nullptr;
	}

	void Renderer::ToggleSwapChain() 
	{
		//m_SwapChain->ToggleVSync();
	}

	void Renderer::MainWindowResized( const IntPoint& NewSize ) 
	{
		Flush();

		IntPoint Size = IntPoint::ComponentWiseMax(NewSize, 1);
		for ( int i = 0; i < NUM_BACKBUFFERS; ++i )
		{
			m_BackBuffers[i].Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		m_SwapChain->GetDesc( &swapChainDesc );
		m_SwapChain->ResizeBuffers( NUM_BACKBUFFERS, Size.X, Size.Y, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags );

		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
		UpdateRenderTargetViews();

#ifndef WITH_EDITOR
		m_MainSceneRenderer->ResizeView(IntPoint(InWidth, InHeight));
#endif
	}

	void Renderer::Tick( float DeltaTime )
	{
		SCOPE_STAT(RendererTick);

		// @TODO: move time to accessible location
		TotalTime += DeltaTime;

		//m_CommandList->Close();
		//m_CommandAllocator->Reset();
		//m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		auto backBuffer = m_BackBuffers[m_CurrentBackbufferIndex];

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );

		m_CommandList->ResourceBarrier( 1, &barrier );

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv( m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			m_CurrentBackbufferIndex, m_RTVDescriporSize );

		m_CommandList->ClearRenderTargetView( rtv, clearColor, 0, nullptr );

		//auto  msaaRenderTarget    = MainSceneRenderer->m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 );
#ifndef WITH_EDITOR
		auto  msaaRenderTarget    = m_MainSceneRenderer->m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 );
#endif

		for (Scene* S : m_AllocatedScenes)
		{
			S->Render(m_CommandList.Get());
		}

#if WITH_EDITOR
		m_CommandList->OMSetRenderTargets(1, &rtv, false, NULL);
		ImGuiRenderer::Get()->Tick( 1, rtv, m_CommandList.Get() );
#else
		// m_CommandList->ResolveSubresource( swapChainBackBuffer, msaaRenderTarget );
		m_CommandList->CopyResource( swapChainBackBuffer, msaaRenderTarget );
#endif

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
		m_CommandList->ResourceBarrier( 1, &barrier );

		//m_CommandList->Close();
		//ID3D12CommandList* const commandLists[] = { m_CommandList.Get() };
		SCOPE_STAT( RendererExecuteCommandList );
		Flush();
		//m_CommandQueue->ExecuteCommandLists( 1, commandLists );
		//Signal();

#if WITH_EDITOR
		ImGuiRenderer::Get()->PostExecuteCommands();
#endif

		m_SwapChain->Present( 0, 0);
		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		//Flush();
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