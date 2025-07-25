#include "DrnPCH.h"
#include "Renderer.h"
#include "BufferedResource.h"

#include "Runtime/Core/Window.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include "Runtime/Core/Application.h"
#include <thread>

LOG_DEFINE_CATEGORY( LogRenderer, "Renderer" );

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Drn
{
	Renderer* Renderer::SingletonInstance = nullptr;

	Renderer::Renderer()
		: m_CommandList(nullptr)
		, m_UploadCommandList(nullptr)
	{
	}

	Renderer::~Renderer()
	{
		if (m_CommandList)
		{
			m_CommandList->ReleaseBufferedResource();
			m_CommandList = nullptr;
		}

		if (m_UploadCommandList)
		{
			m_UploadCommandList->ReleaseBufferedResource();
			m_UploadCommandList = nullptr;
		}
	}

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init( HINSTANCE inhInstance, Window* InMainWindow )
	{
		BufferedResourceManager::Init();
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

		m_RtvIncrementSize = GetD3D12Device()->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
		
		m_SwapChain = std::make_unique<SwapChain>(m_Device.get(), m_MainWindow->GetWindowHandle(), m_CommandQueue.Get(), m_MainWindow->GetWindowSize());

		m_CommandList = new D3D12CommandList(m_Device->GetD3D12Device(), D3D12_COMMAND_LIST_TYPE_DIRECT, NUM_BACKBUFFERS, "RendererDirect");
		m_CommandList->Close();

		m_UploadCommandList = new D3D12CommandList(m_Device->GetD3D12Device(), D3D12_COMMAND_LIST_TYPE_DIRECT, NUM_BACKBUFFERS, "RendererUpload");
		m_UploadCommandList->Close();

		GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
		m_FenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

#if D3D12_Debug_INFO
		m_CommandQueue->SetName(L"MainCommandQueue");
		m_Fence->SetName(L"MainFence");
#endif

		m_CommandList->SetAllocatorAndReset(m_SwapChain->GetBackBufferIndex());
		CommonResources::Init(m_CommandList->GetD3D12CommandList());

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors             = 256;
			desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_SrvHeap.GetAddressOf() ) );
			TempSRVAllocator.Create( Renderer::Get()->GetD3D12Device(), m_SrvHeap.Get() );
		}

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			desc.NumDescriptors             = 256;
			desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_SamplerHeap.GetAddressOf() ) );
			TempSamplerAllocator.Create( Renderer::Get()->GetD3D12Device(), m_SamplerHeap.Get() );
		}

#if D3D12_Debug_INFO
		m_SrvHeap->SetName(L"GlobalSrvHeap");
		m_SamplerHeap->SetName(L"GlobalSamplerHeap");
#endif

#if WITH_EDITOR
		ImGuiRenderer::Get()->Init(m_MainWindow);
#endif

		m_CommandList->Close();
		ID3D12CommandList* const commandLists[] = { m_CommandList->GetD3D12CommandList() };
		m_CommandQueue->ExecuteCommandLists( 1, commandLists );

		Flush();

		tf::Task BeginRender = m_RendererTickTask.emplace( []()
		{
			OPTICK_THREAD_TASK();

			Renderer::Get()->InitRender(Time::GetApplicationDeltaTime());
			Renderer::Get()->UpdateSceneProxyAndResources();
		});

		tf::Task Render = m_RendererTickTask.emplace( [](tf::Subflow& subflow)
		{
			OPTICK_THREAD_TASK();

			for (Scene* S : Renderer::Get()->m_AllocatedScenes)
			{
				for (SceneRenderer* SceneRen : S->m_SceneRenderers)
				{
					tf::Task SceneRenderTask = subflow.composed_of(SceneRen->m_RenderTask);

#if WITH_EDITOR
					subflow.retain(true);
					SceneRenderTask.name("RenderSceneRenderer_" + SceneRen->GetName());
#endif
				}
			}
		});

		tf::Task FinishRender = m_RendererTickTask.emplace( []()
		{
			OPTICK_THREAD_TASK();

			Renderer::Get()->RenderImgui();
			Renderer::Get()->ResolveDisplayBuffer();
			Renderer::Get()->ExecuteCommands();
			Renderer::Get()->m_SwapChain->Present();
		});

		BeginRender.precede(Render);
		Render.precede(FinishRender);

#if WITH_EDITOR
		BeginRender.name( "BeginRender" );
		Render.name( "Render" );
		FinishRender.name( "FinishRender" );
#endif

	}


	uint64_t Renderer::Signal( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue )
	{
		SCOPE_STAT();
		commandQueue->Signal( fence.Get(), ++fenceValue);

		return fenceValue;
	}

	void Renderer::WaitForFenceValue( Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent )
	{
		SCOPE_STAT();

		if (fence->GetCompletedValue() < fenceValue)
		{
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, DWORD_MAX);
		}
	}

	ID3D12GraphicsCommandList2* Renderer::GetCommandList()
	{
		return m_CommandList->GetD3D12CommandList();
	}

	void Renderer::Flush()
	{
		uint64_t fenceValueForSignal = Signal(m_CommandQueue, m_Fence, m_FenceValue);
		WaitForFenceValue(m_Fence, fenceValueForSignal, m_FenceEvent);
	}

	void Renderer::ReportLiveObjects()
	{
		IDXGIDebug1* dxgiDebug;
		DXGIGetDebugInterface1( 0, IID_PPV_ARGS( &dxgiDebug ) );

		dxgiDebug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
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

		SingletonInstance->m_SrvHeap.Reset();
		SingletonInstance->m_SamplerHeap.Reset();

		SingletonInstance->TempSamplerAllocator.Destroy();
		SingletonInstance->TempSRVAllocator.Destroy();

		CommonResources::Shutdown();

		delete SingletonInstance;
		SingletonInstance = nullptr;

		BufferedResourceManager::Get()->Flush();
		BufferedResourceManager::Shutdown();
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
		SCOPE_STAT();

		InitRender(DeltaTime);
		UpdateSceneProxyAndResources();
		RenderSceneRenderers();
		RenderImgui();
		ResolveDisplayBuffer();
		ExecuteCommands();
		m_SwapChain->Present();
	}

	void Renderer::InitRender(float DeltaTime)
	{
		WaitForFenceValue( m_Fence, m_SwapChain->m_FrameFenceValues[m_SwapChain->m_CurrentBackbufferIndex], m_FenceEvent );

		{
			SCOPE_STAT( "CommandlistReset" );
			m_CommandList->SetAllocatorAndReset(m_SwapChain->GetBackBufferIndex());
			m_UploadCommandList->SetAllocatorAndReset(m_SwapChain->GetBackBufferIndex());
		}

		BufferedResourceManager::Get()->Tick(DeltaTime);
		SetHeaps(m_CommandList->GetD3D12CommandList());
	}

	void Renderer::UpdateSceneProxyAndResources()
	{
		for (Scene* S : m_AllocatedScenes)
		{
			S->UpdatePendingProxyAndResources(m_UploadCommandList->GetD3D12CommandList());
		}
	}

	void Renderer::RenderSceneRenderers()
	{
		for (Scene* S : m_AllocatedScenes)
		{
			for (SceneRenderer* SceneRen : S->m_SceneRenderers)
			{
				SceneRen->Render();
			}
		}
	}

	void Renderer::RenderImgui()
	{
#if WITH_EDITOR

		ID3D12Resource* backBuffer = m_SwapChain->GetBackBuffer();
		D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_SwapChain->GetBackBufferHandle();

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );

		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &rtv, false, NULL);

		ImGuiRenderer::Get()->Tick( 1, rtv, m_CommandList->GetD3D12CommandList() );

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );

#endif
	}

	void Renderer::ResolveDisplayBuffer()
	{
#ifndef WITH_EDITOR

		ID3D12Resource* backBuffer = m_SwapChain->GetBackBuffer();

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_MainSceneRenderer->GetViewResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE );
		m_CommandList->ResourceBarrier( 1, &barrier );
		barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST );
		m_CommandList->ResourceBarrier( 1, &barrier );

		m_CommandList->CopyResource(backBuffer, m_MainSceneRenderer->GetViewResource());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_MainSceneRenderer->GetViewResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->ResourceBarrier( 1, &barrier );
		barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT );
		m_CommandList->ResourceBarrier( 1, &barrier );

#endif
	}

	void Renderer::ExecuteCommands()
	{
		uint32 NumCommandLists = 2;
		std::vector<ID3D12CommandList*> CommandLists;

		m_CommandList->Close();
		m_UploadCommandList->Close();

		CommandLists.push_back(m_UploadCommandList->GetD3D12CommandList());

		for (Scene* S : m_AllocatedScenes)
		{
			for (SceneRenderer* SceneRen : S->m_SceneRenderers)
			{
				if (SceneRen->m_CommandList)
				{
					CommandLists.push_back(SceneRen->m_CommandList->GetD3D12CommandList());
					NumCommandLists++;
				}
			}
		}

		CommandLists.push_back(m_CommandList->GetD3D12CommandList());

		m_CommandQueue->ExecuteCommandLists(NumCommandLists, CommandLists.data());
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

	void Renderer::SetHeaps( ID3D12GraphicsCommandList* CommandList )
	{
		SCOPE_STAT();
			
		ID3D12DescriptorHeap* const Descs[2] = { m_SrvHeap.Get(), m_SamplerHeap.Get() };
		m_CommandList->GetD3D12CommandList()->SetDescriptorHeaps(2, Descs);

		//m_UploadCommandList->GetD3D12CommandList()->SetDescriptorHeaps(2, Descs);
	}

}