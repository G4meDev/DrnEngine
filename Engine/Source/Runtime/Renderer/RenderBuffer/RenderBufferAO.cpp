#include "DrnPCH.h"
#include "RenderBufferAO.h"

#define AO_FORMAT DXGI_FORMAT_R8_UNORM
#define AO_SETUP_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT

namespace Drn
{
	RenderBufferAO::RenderBufferAO()
		: RenderBuffer()
		, m_AOTarget(nullptr)
		, m_AOSetupTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		
	}

	RenderBufferAO::~RenderBufferAO()
	{
		if (m_AOTarget)
		{
			m_AOTarget->ReleaseBufferedResource();
		}
		
		if (m_AOSetupTarget)
		{
			m_AOSetupTarget->ReleaseBufferedResource();
		}
	}

	void RenderBufferAO::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		{
			D3D12_DESCRIPTOR_HEAP_DESC AOHeapDesc = {};
			AOHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			AOHeapDesc.NumDescriptors = 2;
			Device->CreateDescriptorHeap( &AOHeapDesc, IID_PPV_ARGS(m_AORtvHeap.ReleaseAndGetAddressOf()) );

			m_AOHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_AORtvHeap->GetCPUDescriptorHandleForHeapStart(), 0, Renderer::Get()->GetRtvIncrementSize());
			m_AOSetupHandle	= CD3DX12_CPU_DESCRIPTOR_HANDLE(m_AORtvHeap->GetCPUDescriptorHandleForHeapStart(), 1, Renderer::Get()->GetRtvIncrementSize());
#if D3D12_Debug_INFO
			m_AORtvHeap->SetName(L"RtvHeap_AO");
#endif
		}
	}

	void RenderBufferAO::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		IntPoint HalfSize = IntPoint::ComponentWiseMax(Size / 2, IntPoint(1));
		m_SetupViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(HalfSize.X), static_cast<float>(HalfSize.Y));


		if (m_AOTarget)
			m_AOTarget->ReleaseBufferedResource();

		if (m_AOSetupTarget)
			m_AOSetupTarget->ReleaseBufferedResource();

		{
			m_AOTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(AO_FORMAT, HalfSize.X, HalfSize.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = AO_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_AOTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_AORtvHeap->GetCPUDescriptorHandleForHeapStart() );

#if D3D12_Debug_INFO
			m_AOTarget->SetName( "AOTarget" );
#endif
		}

		{
			m_AOSetupTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(AO_SETUP_FORMAT, HalfSize.X, HalfSize.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = AO_SETUP_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_AOSetupTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_AOSetupHandle );

#if D3D12_Debug_INFO
			m_AOSetupTarget->SetName( "AOSetupTarget" );
#endif

			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = AO_SETUP_FORMAT;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView(m_AOSetupTarget->GetD3D12Resource(), &Desc, m_AOSetupTarget->GetCpuHandle());
		}
	}

	void RenderBufferAO::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
	}

	void RenderBufferAO::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		//CommandList->RSSetViewports(1, &m_Viewport);
		//CommandList->RSSetScissorRects(1, &m_ScissorRect);
		//
		//CommandList->OMSetRenderTargets( 1, &m_AOHandle, true, nullptr );
	}

	void RenderBufferAO::BindSetup( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_SetupViewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);
		
		CommandList->OMSetRenderTargets( 1, &m_AOSetupHandle, true, nullptr );
	}

	void RenderBufferAO::BindMain( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_SetupViewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);
		
		CommandList->OMSetRenderTargets( 1, &m_AOHandle, true, nullptr );
	}

}