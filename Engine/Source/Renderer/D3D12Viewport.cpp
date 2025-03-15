#include "DrnPCH.h"
#include "D3D12Device.h"
#include "D3D12Viewport.h"
#include "D3D12Adapter.h"
#include "D3D12Queue.h"
#include "D3D12Descriptors.h"

namespace Drn
{
	D3D12Viewport::D3D12Viewport(D3D12Adapter* InAdapter, HWND InWindowHandle, UINT InSizeX, UINT InSizeY, bool InFullScreen, DXGI_FORMAT InPixelFormat)
		: Adapter(InAdapter)
		, WindowHandle(InWindowHandle)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
		, bFullScreen(InFullScreen)
		, PixelFormat(InPixelFormat)
	{
		Adapter->GetViewports().push_back(this);
		Init();
	}

	void D3D12Viewport::Init()
	{
		CommandQueue_Direct = new D3D12Queue(Adapter->GetDevice(), D3D12QueueType::Direct);
		CommandQueue_Copy = new D3D12Queue(Adapter->GetDevice(), D3D12QueueType::Copy);
		CommandQueue_Compute = new D3D12Queue(Adapter->GetDevice(), D3D12QueueType::Compute);

		UINT SwapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1{};

		SwapChainDesc1.Width = SizeX;
		SwapChainDesc1.Height = SizeY;
		SwapChainDesc1.Format = PixelFormat;
		SwapChainDesc1.SampleDesc.Count = 1;
		SwapChainDesc1.SampleDesc.Quality = 0;
		SwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc1.BufferCount = NUM_BACKBUFFERS;
		SwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		SwapChainDesc1.Flags = SwapChainFlags;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullscreenDesc{};
		FullscreenDesc.RefreshRate.Numerator = 0;
		FullscreenDesc.RefreshRate.Denominator = 0;
		FullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		FullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		FullscreenDesc.Windowed = !bFullScreen;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain1;
		VERIFYD3D12RESULT(Adapter->GetFactory()->CreateSwapChainForHwnd(CommandQueue_Direct->CommandQueue.Get(), WindowHandle, &SwapChainDesc1, &FullscreenDesc, nullptr, SwapChain1.GetAddressOf()));

		SwapChain1.As(&SwapChain);

		SwapChainDescriptorRVTRoot = D3D12DescriptorHeap::Create(Adapter->GetDevice(), L"Swap chain descriptor", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_BACKBUFFERS, ED3D12DescriptorHeapFlags::None, false);

		for (UINT n = 0; n < NUM_BACKBUFFERS; n++)
		{
			ID3D12Resource* RenderTargetPtr = BackBuffers[n].get();
			SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargetPtr));

			wchar_t name[25] = {};
			swprintf_s(name, L"Render target %u", n);
			RenderTargetPtr->SetName(name);

			D3D12DescriptorHeap* RVT = new D3D12DescriptorHeap(SwapChainDescriptorRVTRoot, n, 1);
			SwapChainDescriptorRVT[n].reset(RVT);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = PixelFormat;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			Adapter->GetD3DDevice()->CreateRenderTargetView(RenderTargetPtr, &rtvDesc, RVT->GetCpuHandle());
		}

		BackBufferIndex = SwapChain->GetCurrentBackBufferIndex();

		
	}
}