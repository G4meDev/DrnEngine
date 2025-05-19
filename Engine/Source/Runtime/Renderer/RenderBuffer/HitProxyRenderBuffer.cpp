#include "DrnPCH.h"
#include "HitProxyRenderBuffer.h"

#if WITH_EDITOR

namespace Drn
{
	HitProxyRenderBuffer::HitProxyRenderBuffer()
		: RenderBuffer()
		, m_GuidTarget(nullptr)
		, m_DepthTarget(nullptr)
	{
		m_GuidClearValue.Format   = GBUFFER_GUID_FORMAT;
		m_GuidClearValue.Color[0] = 0.0f;
		m_GuidClearValue.Color[1] = 0.0f;
		m_GuidClearValue.Color[2] = 0.0f;
		m_GuidClearValue.Color[3] = 0.0f;

		m_DepthClearValue.Format = DEPTH_FORMAT;
		m_DepthClearValue.DepthStencil = { 0, 0 };
	}

	HitProxyRenderBuffer::~HitProxyRenderBuffer()
	{
		if (m_GuidTarget)
		{
			m_GuidTarget->ReleaseBufferedResource();
		}

		if (m_DepthTarget)
		{
			m_DepthTarget->ReleaseBufferedResource();
		}
	}

	void HitProxyRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		{
			D3D12_DESCRIPTOR_HEAP_DESC GuidHeapDesc = {};
			GuidHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			GuidHeapDesc.NumDescriptors = 1;
			Device->CreateDescriptorHeap( &GuidHeapDesc, IID_PPV_ARGS(m_GuidDescriptorHeap.ReleaseAndGetAddressOf()) );
		}

		{
			D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
			DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			DepthHeapDesc.NumDescriptors = 1;
			Device->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DepthHeap.ReleaseAndGetAddressOf()) );
		}
	}

	void HitProxyRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		if (m_GuidTarget)
			m_GuidTarget->ReleaseBufferedResource();

		m_GuidTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_GUID_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
			D3D12_RESOURCE_STATE_RENDER_TARGET, m_GuidClearValue);

		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
		RenderTargetViewDesc.Format = GBUFFER_GUID_FORMAT;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;

		Device->CreateRenderTargetView( m_GuidTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_GuidDescriptorHeap->GetCPUDescriptorHandleForHeapStart() );
		//Device->CreateRenderTargetView( m_GuidTarget.Get(), &rtv, CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), 1, RTVDescriporSize) );

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

		if (m_DepthTarget)
			m_DepthTarget->ReleaseBufferedResource();
		
		m_DepthTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DEPTH_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, m_DepthClearValue);
		
		D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
		DepthViewDesc.Format = DEPTH_FORMAT;
		DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DepthViewDesc.Texture2D.MipSlice = 0;
		
		Device->CreateDepthStencilView( m_DepthTarget->GetD3D12Resource(), &DepthViewDesc, m_DepthHeap->GetCPUDescriptorHandleForHeapStart() );

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if D3D12_Debug_INFO
		m_GuidTarget->SetName( "HitProxy_GuidTarget" );
		m_DepthTarget->SetName( "HitProxy_DepthTarget" );
#endif


	}

	void HitProxyRenderBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->ClearRenderTargetView( m_GuidDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_GuidClearValue.Color, 0, nullptr );
		CommandList->ClearDepthStencilView( m_DepthHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 0, 0, 0, nullptr );
	}

	void HitProxyRenderBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->OMSetRenderTargets( 1, &m_GuidDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, &m_DepthHeap->GetCPUDescriptorHandleForHeapStart() );
	}

}
#endif