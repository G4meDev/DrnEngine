#include "DrnPCH.h"
#include "TonemapRenderBuffer.h"

namespace Drn
{
	TonemapRenderBuffer::TonemapRenderBuffer()
		: RenderBuffer()
		, m_TonemapTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		m_TonemapClearValue.Format   = DISPLAY_OUTPUT_FORMAT;
		m_TonemapClearValue.Color[0] = 0.0f;
		m_TonemapClearValue.Color[1] = 0.0f;
		m_TonemapClearValue.Color[2] = 0.0f;
		m_TonemapClearValue.Color[3] = 0.0f;

	}

	TonemapRenderBuffer::~TonemapRenderBuffer()
	{
		if (m_TonemapTarget)
		{
			m_TonemapTarget->ReleaseBufferedResource();
		}
	}

	void TonemapRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		{
			D3D12_DESCRIPTOR_HEAP_DESC TonemapHeapDesc = {};
			TonemapHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			TonemapHeapDesc.NumDescriptors = 1;
			Device->CreateDescriptorHeap( &TonemapHeapDesc, IID_PPV_ARGS(m_TonemapRtvHeap.ReleaseAndGetAddressOf()) );
		}
	}

	void TonemapRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		if (m_TonemapTarget)
			m_TonemapTarget->ReleaseBufferedResource();

		m_TonemapTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DISPLAY_OUTPUT_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
			D3D12_RESOURCE_STATE_RENDER_TARGET, m_TonemapClearValue);

		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
		RenderTargetViewDesc.Format = DISPLAY_OUTPUT_FORMAT;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;

		Device->CreateRenderTargetView( m_TonemapTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_TonemapRtvHeap->GetCPUDescriptorHandleForHeapStart() );

#if D3D12_Debug_INFO
		m_TonemapTarget->SetName( "TonemapTarget" );
#endif
	}

	void TonemapRenderBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->ClearRenderTargetView( m_TonemapRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_TonemapClearValue.Color, 0, nullptr );
	}

	void TonemapRenderBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);

		CommandList->OMSetRenderTargets( 1, &m_TonemapRtvHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr );
	}

}