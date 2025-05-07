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
#if D3D12_DEBUG_LAYER
		ComPtr<ID3D12Debug> debugInterface;
		D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) );
		debugInterface->EnableDebugLayer();
#endif

		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;
#if D3D12_DEBUG_LAYER
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		bool UseWarp = false;

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

		D3D12CreateDevice(dxgiAdapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf()));

		DXGI_ADAPTER_DESC3 Desc;
		dxgiAdapter4->GetDesc3(&Desc);

		LOG( LogRenderer, Info, "%s", StringHelper::ws2s(Desc.Description).c_str() );

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
 
			pInfoQueue->PushStorageFilter(&NewFilter);
		}
#endif

		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = { };
		CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		m_Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));

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

		m_TearingSupported = CheckTearingSupport();
		swapChainDesc.Flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		dxgiFactory->CreateSwapChainForHwnd( m_CommandQueue.Get(), m_MainWindow->GetWindowHandle(), &swapChainDesc, nullptr, nullptr, &swapChain1 );
		swapChain1.As( &m_SwapChain );
		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = NUM_BACKBUFFERS;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap));
		UpdateRenderTargetViews();

		for (int i = 0; i < NUM_BACKBUFFERS; i++)
		{
			m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator[i].GetAddressOf()));
		}
		m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator[m_CurrentBackbufferIndex].Get(), NULL, IID_PPV_ARGS(m_CommandList.GetAddressOf()));

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

	bool Renderer::CheckTearingSupport()
	{
		BOOL allowTearing = FALSE;

		ComPtr<IDXGIFactory4> factory4;
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
		{
			ComPtr<IDXGIFactory5> factory5;
			if (SUCCEEDED(factory4.As(&factory5)))
			{
				if (FAILED(factory5->CheckFeatureSupport(
					DXGI_FEATURE_PRESENT_ALLOW_TEARING, 
					&allowTearing, sizeof(allowTearing))))
				{
					allowTearing = FALSE;
				}
			}
		}

		return allowTearing == TRUE;
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

	uint64_t Renderer::Signal( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue )
	{
		uint64_t fenceValueForSignal = ++fenceValue;
		commandQueue->Signal( fence.Get(), fenceValueForSignal );

		return fenceValueForSignal;
	}

	void Renderer::WaitForFenceValue( Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent )
	{
		if (fence->GetCompletedValue() < fenceValue)
		{
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, DWORD_MAX);
		}
	}

	void Renderer::Flush( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent )
	{
		uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
		WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
	}

	void Renderer::Flush()
	{
		Flush(m_CommandQueue, m_Fence, m_FenceValue, m_FenceEvent);
	}

	// void Renderer::Flush()
	//{
	//	SCOPE_STAT( RendererFlush );
	//
	//	//Renderer::Get()->GetCommandList()->Close();
	//	//ID3D12CommandList* const CommandLists[1] = { m_CommandList.Get() };
	//	//m_CommandQueue->ExecuteCommandLists(1, CommandLists);
	//
	//	//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	//
	//	uint64_t fenceValueForSignal = Signal();
	//	WaitForFenceValue(fenceValueForSignal);
	//
	//	//m_CommandList->Reset( m_CommandAllocator.Get(), NULL );
	//}


	void Renderer::ReportLiveObjects()
	{
		IDXGIDebug1* dxgiDebug;
		DXGIGetDebugInterface1( 0, IID_PPV_ARGS( &dxgiDebug ) );

		dxgiDebug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL );
		dxgiDebug->Release();
	}

	void Renderer::Shutdown()
	{
		LOG(LogRenderer, Info, "Renderer shutdown.");
		SingletonInstance->Flush();

#if WITH_EDITOR
		ImGuiRenderer::Get()->Shutdown();
#endif

		for (Scene* S : SingletonInstance->m_AllocatedScenes)
		{
			delete S;
		}

		CloseHandle(SingletonInstance->m_FenceEvent);

		SingletonInstance->ReportLiveObjects();

		delete SingletonInstance;
		SingletonInstance = nullptr;
	}

	void Renderer::MainWindowResized( const IntPoint& NewSize ) 
	{
		Flush();

		IntPoint Size = IntPoint::ComponentWiseMax(NewSize, 1);
		for ( int i = 0; i < NUM_BACKBUFFERS; ++i )
		{
			m_BackBuffers[i].Reset();
			m_FrameFenceValues[i] = m_FrameFenceValues[m_CurrentBackbufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		m_SwapChain->GetDesc( &swapChainDesc );
		m_SwapChain->ResizeBuffers( NUM_BACKBUFFERS, Size.X, Size.Y, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags );

		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
		UpdateRenderTargetViews();

#ifndef WITH_EDITOR
		m_MainSceneRenderer->ResizeView(NewSize);
#endif
	}

	void Renderer::Tick( float DeltaTime )
	{
		SCOPE_STAT(RendererTick);

		// @TODO: move time to accessible location
		TotalTime += DeltaTime;

		auto commandAllocator = m_CommandAllocator[m_CurrentBackbufferIndex];
		commandAllocator->Reset();
		m_CommandList->Reset(commandAllocator.Get(), nullptr);

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		auto backBuffer = m_BackBuffers[m_CurrentBackbufferIndex];

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );

		m_CommandList->ResourceBarrier( 1, &barrier );

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv( m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			m_CurrentBackbufferIndex, m_RTVDescriporSize );

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
		barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST );
		m_CommandList->ResourceBarrier( 1, &barrier );

		m_CommandList->CopyResource(backBuffer.Get(), m_MainSceneRenderer->m_ColorTarget.Get());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_MainSceneRenderer->m_ColorTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->ResourceBarrier( 1, &barrier );
		barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->ResourceBarrier( 1, &barrier );

#endif


		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
		m_CommandList->ResourceBarrier( 1, &barrier );

		SCOPE_STAT( RendererExecuteCommandList );
		
		m_CommandList->Close();
		ID3D12CommandList* const commandLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists( 1, commandLists );

#if WITH_EDITOR
		ImGuiRenderer::Get()->PostExecuteCommands();
#endif

		m_FrameFenceValues[m_CurrentBackbufferIndex] = Signal( m_CommandQueue, m_Fence, m_FenceValue );

		UINT syncInterval = m_Vsync ? 1 : 0;
		UINT presentFlags = m_TearingSupported && !m_Vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

		m_SwapChain->Present( syncInterval, presentFlags);
		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		SCOPE_STAT( RendererWait );
		WaitForFenceValue( m_Fence, m_FrameFenceValues[m_CurrentBackbufferIndex], m_FenceEvent );
		//std::this_thread::sleep_for(std::chrono::milliseconds(2));
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