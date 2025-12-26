#include "DrnPCH.h"
#include "SwapChain.h"
#include "Runtime/Core/Window.h"
#include "Runtime/Renderer/Renderer.h"

LOG_DEFINE_CATEGORY( LogSwapChain, "SwapChain" );

namespace Drn
{
	SwapChain::SwapChain(Device* InDevice, HWND WindowHandle, ID3D12CommandQueue* CommandQueue, const IntPoint& Size)
		: m_Device(InDevice)
		, m_SwapChain(nullptr)
		, m_Size(Size)
		, m_WindowedMode(true)
		, m_CommandQueue(CommandQueue)
	{
#if D3D12_DEBUG_LAYER
		Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
		D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) );
		debugInterface->EnableDebugLayer();
#endif

		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;
#if D3D12_DEBUG_LAYER
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1;
		Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width                 = Size.X;
		swapChainDesc.Height                = Size.Y;
		swapChainDesc.Format                = DISPLAY_OUTPUT_FORMAT;
		swapChainDesc.Stereo                = FALSE;
		swapChainDesc.SampleDesc            = { 1, 0 };
		swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount           = NUM_BACKBUFFERS;
		swapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;

		m_TearingSupported = CheckTearingSupport();
		swapChainDesc.Flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
		dxgiFactory->CreateSwapChainForHwnd( CommandQueue, WindowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1 );
		swapChain1.As( &m_SwapChain );

		dxgiFactory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_ALT_ENTER);
		//if (m_TearingSupported)
		//{
		//	dxgiFactory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_ALT_ENTER);
		//}

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = NUM_BACKBUFFERS;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		m_Device->GetD3D12Device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap));
		UpdateRenderTargetViews();
		
		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

#if D3D12_Debug_INFO
		m_RTVDescriptorHeap->SetName(L"SwapChainRtvHeap");
#endif
		LOG( LogSwapChain, Info, "Successfully created swap chain with %i back buffers", NUM_BACKBUFFERS );
	}

	SwapChain::~SwapChain()
	{
		LOG( LogSwapChain, Info, "destroying swap chain" );
		m_SwapChain->SetFullscreenState(FALSE, nullptr);
	}

	void SwapChain::Present()
	{
		SCOPE_STAT();
		m_FrameFenceValues[m_CurrentBackbufferIndex] = Renderer::Get()->GetFence()->Signal();
		
		UINT syncInterval = m_Vsync ? 1 : 0;
		UINT presentFlags = m_TearingSupported && m_WindowedMode && !m_Vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

		//OPTICK_GPU_FLIP( m_SwapChain.Get() );
		m_SwapChain->Present( syncInterval, presentFlags);
		m_CurrentBackbufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void SwapChain::Resize( const IntPoint& NewSize )
	{
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

		BOOL bFullScreen;
		m_SwapChain->GetFullscreenState(&bFullScreen, nullptr);
		m_WindowedMode = !bFullScreen;

		UpdateRenderTargetViews();

	}

	bool SwapChain::CheckTearingSupport()
	{
#if DISABLE_TEARING_SUPPORT
		return false;
#else
		BOOL allowTearing = FALSE;

		Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
		{
			Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
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
#endif
	}

	void SwapChain::SetFullScreen( bool bFullScreen )
	{
		if(m_TearingSupported)
		{
			Renderer::Get()->GetMainWindow()->SetBorderlessFullScreen(bFullScreen);
		}
		else
		{
			RECT FullScreenRect;
			if (!GetContainingRect(FullScreenRect))
			{
				drn_check(false);
			}

			uint32 Width = FullScreenRect.right - FullScreenRect.left;
			uint32 Height = FullScreenRect.bottom - FullScreenRect.top;

			DXGI_MODE_DESC D = {};
			D.Width = Width;
			D.Height = Height;
			D.Format = DISPLAY_OUTPUT_FORMAT;
			D.Scaling = DXGI_MODE_SCALING_STRETCHED;
			D.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			m_SwapChain->ResizeTarget(&D);

			m_SwapChain->SetFullscreenState(bFullScreen, NULL);

			if (!bFullScreen)
			{
				Renderer::Get()->GetMainWindow()->SetBorderlessFullScreen(false);
			}
		}
	}

	bool SwapChain::GetContainingRect( RECT& OutRect )
	{
		if (m_SwapChain)
		{
			TRefCountPtr<IDXGIOutput> pOutput;
			HRESULT HRes = m_SwapChain->GetContainingOutput(pOutput.GetInitReference());

			if (HRes == S_OK)
			{
				DXGI_OUTPUT_DESC Desc;
				HRes = pOutput->GetDesc(&Desc);

				if (HRes == S_OK)
				{
					OutRect = Desc.DesktopCoordinates;
					return true;
				}
			}
		}

		return false;
	}

	void SwapChain::UpdateRenderTargetViews()
	{
		m_RTVDescriporSize = m_Device->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
 
		for (int i = 0; i < NUM_BACKBUFFERS; ++i)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
 
			m_Device->GetD3D12Device()->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
			m_BackBuffers[i] = backBuffer;
			rtvHandle.Offset(m_RTVDescriporSize);
		}
	}

}