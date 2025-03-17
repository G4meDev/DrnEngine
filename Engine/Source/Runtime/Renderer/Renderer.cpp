#include "DrnPCH.h"
#include "Renderer.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"
#include "D3D12Scene.h"
#include "D3D12Queue.h"
#include "D3D12Descriptors.h"

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	Renderer* Renderer::SingletonInstance;

	void Renderer::CreateMainScene()
	{
		Get()->MainScene = new D3D12Scene(Adapter, GetMainWindow()->GetWindowHandle(), GetMainWindow()->GetSizeX(), GetMainWindow()->GetSizeY(), false, DISPLAY_OUTPUT_FORMAT);
	}

	void Renderer::CreateResources()
	{
		CommandQueue = std::make_unique<D3D12Queue>(D3D12Queue(Adapter->GetDevice(), D3D12QueueType::Direct));
		CommandAllocator = std::make_unique<D3D12CommandAllocator>(D3D12CommandAllocator(Adapter->GetDevice(), D3D12QueueType::Direct));

		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator->CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList)));
		VERIFYD3D12RESULT(CommandList->Close());
	}

	void Renderer::CreateSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1{};

		SwapChainDesc1.Width = MainWindow->GetSizeX();
		SwapChainDesc1.Height = MainWindow->GetSizeY();
		SwapChainDesc1.Format = DISPLAY_OUTPUT_FORMAT;
		SwapChainDesc1.SampleDesc.Count = 1;
		SwapChainDesc1.SampleDesc.Quality = 0;
		SwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc1.BufferCount = NUM_BACKBUFFERS;
		SwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		SwapChainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullscreenDesc{};
		FullscreenDesc.RefreshRate.Numerator = 0;
		FullscreenDesc.RefreshRate.Denominator = 0;
		FullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		FullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		FullscreenDesc.Windowed = true;
		//FullscreenDesc.Windowed = !bFullScreen;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain1;
		VERIFYD3D12RESULT(Adapter->GetFactory()->CreateSwapChainForHwnd(Renderer::Get()->GetCommandQueue(), GetMainWindow()->GetWindowHandle(), &SwapChainDesc1, &FullscreenDesc, nullptr, SwapChain1.GetAddressOf()));

		SwapChain1.As(&SwapChain);


		SwapChainDescriptorRVTRoot = new D3D12DescriptorHeap(Adapter->GetDevice(), NUM_BACKBUFFERS, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, ED3D12DescriptorHeapFlags::None, false);

		for (UINT n = 0; n < NUM_BACKBUFFERS; n++)
		{
			ID3D12Resource* RenderTargetPtr;
			SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargetPtr));

			wchar_t name[25] = {};
			swprintf_s(name, L"Render target %u", n);
			RenderTargetPtr->SetName(name);

			BackBuffers[n] = std::shared_ptr<ID3D12Resource>(RenderTargetPtr);

			D3D12DescriptorHeap* RVT = new D3D12DescriptorHeap(SwapChainDescriptorRVTRoot);
			SwapChainDescriptorRVT[n] = std::shared_ptr<D3D12DescriptorHeap>(RVT);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = DISPLAY_OUTPUT_FORMAT;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			Adapter->GetD3DDevice()->CreateRenderTargetView(RenderTargetPtr, &rtvDesc, RVT->GetCpuHandle());
		}

		BackBufferIndex = SwapChain->GetCurrentBackBufferIndex();


		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf())));
		FenceValue = 1;

		FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (FenceEvent == nullptr)
		{
			VERIFYD3D12RESULT(HRESULT_FROM_WIN32(GetLastError()));
		}

		WaitForPreviousFrame();
	}

	void Renderer::ResetResources()
	{
		VERIFYD3D12RESULT(CommandAllocator->CommandAllocator->Reset());
		VERIFYD3D12RESULT(CommandList->Reset(CommandAllocator->CommandAllocator.Get(), nullptr));
	}

	void Renderer::BindFrontBuffer()
	{
		ID3D12Resource* BackBufferResource = BackBuffers[BackBufferIndex].get();
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BackBufferResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = SwapChainDescriptorRVT[BackBufferIndex]->GetCpuHandle();
		CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	}

	void Renderer::ExecuteCommandList()
	{
		ID3D12Resource* BackBufferResource = BackBuffers[BackBufferIndex].get();

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BackBufferResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		VERIFYD3D12RESULT(CommandList->Close());

		ID3D12CommandList* ppCommandLists[] = { CommandList };
		Renderer::Get()->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		ImGuiRenderer::Get()->PostExecuteCommands();

		VERIFYD3D12RESULT(SwapChain->Present(1, 0));

		//WaitForPreviousFrame();
	}

	void Renderer::WaitForPreviousFrame()
	{
		const UINT64 fence = FenceValue;
		VERIFYD3D12RESULT(Renderer::Get()->GetCommandQueue()->Signal(Fence.Get(), fence));
		FenceValue++;

		if (Fence->GetCompletedValue() < fence)
		{
			VERIFYD3D12RESULT(Fence->SetEventOnCompletion(fence, FenceEvent));
			WaitForSingleObject(FenceEvent, INFINITE);
		}

		BackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
	}

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init(HINSTANCE inhInstance)
	{
		std::cout << "Renderer start!" << std::endl;
		SingletonInstance = new Renderer();

		Get()->Adapter = new D3D12Adapter();
		Get()->CreateResources();

		Get()->MainWindow = std::make_unique<Window>(inhInstance, Renderer::Get()->Adapter, std::wstring(L"Untitled window"));
		Get()->CreateMainScene();

		Get()->CreateSwapChain();

		ImGuiRenderer::Get()->Init();
	}

	void Renderer::Shutdown()
	{
		std::cout << "Renderer shutdown!" << std::endl;
	}

	void Renderer::Tick(float DeltaTime)
	{
		ResetResources();
		MainWindow->Tick(DeltaTime);
		MainScene->Tick(DeltaTime);

		BindFrontBuffer();
		ImGuiRenderer::Get()->Tick(DeltaTime);
		ExecuteCommandList();
	}

	void Renderer::CreateCommandQueue(D3D12Device* Device, const D3D12_COMMAND_QUEUE_DESC& Desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>& OutCommandQueue)
	{
		VERIFYD3D12RESULT(Device->GetD3DDevice()->CreateCommandQueue(&Desc, IID_PPV_ARGS(OutCommandQueue.GetAddressOf())));
	}
}