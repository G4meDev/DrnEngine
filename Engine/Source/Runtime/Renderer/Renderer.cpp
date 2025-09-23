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
		LOG(LogRenderer, Info, "Renderer shutdown.");

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

		if (m_StaticSamplersBuffer)
		{
			m_StaticSamplersBuffer->ReleaseBufferedResource();
			m_StaticSamplersBuffer = nullptr;
		}

		m_BindlessSamplerHeapAllocator.Free(m_BindlessLinearSamplerCpuHandle, m_BindlessLinearSamplerGpuHandle);

		//SingletonInstance->Flush();

#if WITH_EDITOR
		ImGuiRenderer::Get()->Shutdown();
#endif

		for (Scene* S : SingletonInstance->m_AllocatedScenes)
		{
			S->Release();
		}

		CloseHandle(SingletonInstance->m_FenceEvent);
		CommonResources::Shutdown();

		BufferedResourceManager::Get()->Flush();
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

#if D3D12_GPU_VALIDATION
		Microsoft::WRL::ComPtr<ID3D12Debug> spDebugController0;
		Microsoft::WRL::ComPtr<ID3D12Debug1> spDebugController1;
		D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0));
		spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1));
		spDebugController1->SetEnableGPUBasedValidation(true);
#endif

		m_Device = std::make_unique<Device>();

		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = { };
		CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		m_Device->GetD3D12Device()->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));

		m_RtvIncrementSize = GetD3D12Device()->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
		m_DsvIncrementSize = GetD3D12Device()->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
		m_SrvIncrementSize = GetD3D12Device()->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
		m_SamplerIncrementSize = GetD3D12Device()->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
		
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

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors             = 2048;
			desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_BindlessSrvHeap.GetAddressOf() ) );
			m_BindlessSrvHeapAllocator.Create( Renderer::Get()->GetD3D12Device(), m_BindlessSrvHeap.Get() );
		}

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			desc.NumDescriptors             = 256;
			desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_BindlessSamplerHeap.GetAddressOf() ) );
			m_BindlessSamplerHeapAllocator.Create( Renderer::Get()->GetD3D12Device(), m_BindlessSamplerHeap.Get() );
		}

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		std::vector<D3D12_ROOT_PARAMETER> rootParameters;

		D3D12_ROOT_PARAMETER ViewBufferParam = {};
		ViewBufferParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		ViewBufferParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		ViewBufferParam.Constants.Num32BitValues = 32;
		ViewBufferParam.Constants.ShaderRegister = 0;
		ViewBufferParam.Constants.RegisterSpace = 0;

		rootParameters.push_back(ViewBufferParam);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription(rootParameters.size(), rootParameters.data(), 0, nullptr, rootSignatureFlags );
		
		ID3DBlob* pSerializedRootSig;
		ID3DBlob* pRootSigError;
		HRESULT Result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &pSerializedRootSig, &pRootSigError);
		if ( FAILED(Result) )
		{
			if ( pRootSigError )
			{
				LOG(LogMaterial, Error, "shader signature serialization failed. %s", (char*)pRootSigError->GetBufferPointer());
				pRootSigError->Release();
			}
		
			if (pSerializedRootSig)
			{
				pSerializedRootSig->Release();
				pSerializedRootSig = nullptr;
			}
		
			return;
		}
		
		
		Renderer::Get()->GetD3D12Device()->CreateRootSignature(NULL, pSerializedRootSig->GetBufferPointer(),
			pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_BindlessRootSinature.GetAddressOf()));

#if WITH_EDITOR
		m_BindlessSrvHeap->SetName(L"BindlessSrvHeap");
		m_BindlessSamplerHeap->SetName(L"BindlessSamplerHeap");

		m_BindlessRootSinature->SetName(L"BindlessRootSignature");
#endif

		{
			m_BindlessSamplerHeapAllocator.Alloc(&m_BindlessLinearSamplerCpuHandle, &m_BindlessLinearSamplerGpuHandle);
			D3D12_SAMPLER_DESC SamplerDesc = {};
			//SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
			SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.MinLOD = 0.0f;
			SamplerDesc.MaxLOD = FLT_MAX;
			SamplerDesc.MaxAnisotropy = 16;
			GetD3D12Device()->CreateSampler(&SamplerDesc, m_BindlessLinearSamplerCpuHandle);

			m_StaticSamplers.LinearSampler = GetBindlessSamplerIndex(m_BindlessLinearSamplerGpuHandle);
		}

		{
			m_BindlessSamplerHeapAllocator.Alloc(&m_BindlessPointSamplerCpuHandle, &m_BindlessPointSamplerGpuHandle);
			D3D12_SAMPLER_DESC SamplerDesc = {};
			//SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
			SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.MinLOD = 0.0f;
			SamplerDesc.MaxLOD = FLT_MAX;
			SamplerDesc.MaxAnisotropy = 16;
			GetD3D12Device()->CreateSampler(&SamplerDesc, m_BindlessPointSamplerCpuHandle);

			m_StaticSamplers.PointSampler = GetBindlessSamplerIndex(m_BindlessPointSamplerGpuHandle);
		}

		{
			m_BindlessSamplerHeapAllocator.Alloc(&m_BindlessLinearCompLessSamplerCpuHandle, &m_BindlessLinearCompLessSamplerGpuHandle);
			D3D12_SAMPLER_DESC SamplerDesc = {};
			SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			SamplerDesc.BorderColor[0] = SamplerDesc.BorderColor[1] = SamplerDesc.BorderColor[2] = SamplerDesc.BorderColor[3] = 1.0f;
			SamplerDesc.MinLOD = 0.0f;
			SamplerDesc.MaxLOD = FLT_MAX;
			GetD3D12Device()->CreateSampler(&SamplerDesc, m_BindlessLinearCompLessSamplerCpuHandle);

			m_StaticSamplers.LinearCompLessSampler = GetBindlessSamplerIndex(m_BindlessLinearCompLessSamplerGpuHandle);
		}

		{
			m_BindlessSamplerHeapAllocator.Alloc(&m_BindlessLinearClampSamplerCpuHandle, &m_BindlessLinearClampSamplerGpuHandle);
			D3D12_SAMPLER_DESC SamplerDesc = {};
			SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.MinLOD = 0.0f;
			SamplerDesc.MaxLOD = FLT_MAX;
			GetD3D12Device()->CreateSampler(&SamplerDesc, m_BindlessLinearClampSamplerCpuHandle);

			m_StaticSamplers.LinearClampSampler = GetBindlessSamplerIndex(m_BindlessLinearClampSamplerGpuHandle);
		}

		{
			m_BindlessSamplerHeapAllocator.Alloc(&m_BindlessPointClampSamplerCpuHandle, &m_BindlessPointClampSamplerGpuHandle);
			D3D12_SAMPLER_DESC SamplerDesc = {};
			SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.MinLOD = 0.0f;
			SamplerDesc.MaxLOD = FLT_MAX;
			GetD3D12Device()->CreateSampler(&SamplerDesc, m_BindlessPointClampSamplerCpuHandle);

			m_StaticSamplers.PointClampSampler = GetBindlessSamplerIndex(m_BindlessPointClampSamplerGpuHandle);
		}

		m_StaticSamplersBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_StaticSamplersBuffer->SetName("StaticSamplerBuffer");
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_StaticSamplersBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_StaticSamplersBuffer->GetCpuHandle());

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_StaticSamplersBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_StaticSamplers, sizeof(StaticSamplers));
		m_StaticSamplersBuffer->GetD3D12Resource()->Unmap(0, nullptr);


		CommonResources::Init(m_CommandList->GetD3D12CommandList());

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
					SceneRen->Render();

//					tf::Task SceneRenderTask = subflow.composed_of(SceneRen->m_RenderTask);
//
//#if WITH_EDITOR
//					subflow.retain(true);
//					SceneRenderTask.name("RenderSceneRenderer_" + SceneRen->GetName());
//#endif
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

	uint32 Renderer::GetBindlessSrvIndex( D3D12_GPU_DESCRIPTOR_HANDLE Handle )
	{
		return (Handle.ptr - m_BindlessSrvHeap->GetGPUDescriptorHandleForHeapStart().ptr) / m_SrvIncrementSize;
	}

	uint32 Renderer::GetBindlessSamplerIndex( D3D12_GPU_DESCRIPTOR_HANDLE Handle )
	{
		return (Handle.ptr - m_BindlessSamplerHeap->GetGPUDescriptorHandleForHeapStart().ptr) / m_SamplerIncrementSize;
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
		if(SingletonInstance)
		{
			delete SingletonInstance;
			SingletonInstance = nullptr;
		}
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
		SCOPE_STAT( "InitRender" );

		{
			SCOPE_STAT( "WaitForOnFlightCommands" );
			WaitForFenceValue( m_Fence, m_SwapChain->m_FrameFenceValues[m_SwapChain->m_CurrentBackbufferIndex], m_FenceEvent );
		}

		{
			SCOPE_STAT( "CommandlistReset" );
			m_CommandList->SetAllocatorAndReset(m_SwapChain->GetBackBufferIndex());
			m_UploadCommandList->SetAllocatorAndReset(m_SwapChain->GetBackBufferIndex());
		}

		BufferedResourceManager::Get()->Tick(DeltaTime);
		SetBindlessHeaps(m_CommandList->GetD3D12CommandList());
	}

	void Renderer::UpdateSceneProxyAndResources()
	{
		SCOPE_STAT( "UpdateSceneProxyAndResources" );

		for (Scene* S : m_AllocatedScenes)
		{
			S->UpdatePendingProxyAndResources(m_UploadCommandList->GetD3D12CommandList());
		}
	}

	void Renderer::RenderSceneRenderers()
	{
		SCOPE_STAT( "RenderSceneRenderers" );

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
		SCOPE_STAT( "RenderImgui" );

#if WITH_EDITOR

		ID3D12Resource* backBuffer = m_SwapChain->GetBackBuffer();
		D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_SwapChain->GetBackBufferHandle();

		//ResourceStateTracker::Get()->TransiationResource( backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET );
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );
		//ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &rtv, false, NULL);
		ImGuiRenderer::Get()->Tick( Time::GetApplicationDeltaTime(), rtv, m_CommandList->GetD3D12CommandList() );

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );

#endif
	}

	void Renderer::ResolveDisplayBuffer()
	{
		SCOPE_STAT( "ResolveDisplayBuffer" );

#ifndef WITH_EDITOR

		ID3D12Resource* backBuffer = m_SwapChain->GetBackBuffer();
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );

		ResourceStateTracker::Get()->TransiationResource(m_MainSceneRenderer->GetViewResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		m_CommandList->GetD3D12CommandList()->CopyResource(backBuffer, m_MainSceneRenderer->GetViewResource());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition( backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );

#endif
	}

	void Renderer::ExecuteCommands()
	{
		SCOPE_STAT( "ExecuteCommands" );

		uint32 NumCommandLists = 2;
		std::vector<ID3D12CommandList*> CommandLists;

		m_CommandList->Close();
		m_UploadCommandList->Close();

		{
			SCOPE_STAT( "E1" );
			CommandLists.push_back(m_UploadCommandList->GetD3D12CommandList());
			m_CommandQueue->ExecuteCommandLists(1, CommandLists.data());
		}
		//CommandLists.push_back(m_UploadCommandList->GetD3D12CommandList());

		for (Scene* S : m_AllocatedScenes)
		{
			for (SceneRenderer* SceneRen : S->m_SceneRenderers)
			{
				if (SceneRen->m_CommandList)
				{
					//CommandLists.push_back(SceneRen->m_CommandList->GetD3D12CommandList());
					//NumCommandLists++;

					{
						SCOPE_STAT( "E2" );
						CommandLists.clear();
						CommandLists.push_back(SceneRen->m_CommandList->GetD3D12CommandList());
						m_CommandQueue->ExecuteCommandLists(1, CommandLists.data());
					}
				}
			}
		}

		//CommandLists.push_back(m_CommandList->GetD3D12CommandList());

		{
			SCOPE_STAT( "E3" );
			CommandLists.clear();
			CommandLists.push_back(m_CommandList->GetD3D12CommandList());
			m_CommandQueue->ExecuteCommandLists(1, CommandLists.data());
		}
		
		//m_CommandQueue->ExecuteCommandLists(NumCommandLists, CommandLists.data());
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

	void Renderer::SetBindlessHeaps( ID3D12GraphicsCommandList* CommandList )
	{
		SCOPE_STAT();
	
		ID3D12DescriptorHeap* const Descs[2] = { m_BindlessSrvHeap.Get(), m_BindlessSamplerHeap.Get() };
		CommandList->SetDescriptorHeaps(2, Descs);
	}

}