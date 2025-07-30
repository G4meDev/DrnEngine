#include "DrnPCH.h"
#include "EditorPrimitiveRenderBuffer.h"

namespace Drn
{
	EditorPrimitiveRenderBuffer::EditorPrimitiveRenderBuffer()
		: RenderBuffer()
		, m_ColorTarget(nullptr)
		, m_DepthTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		m_ColorClearValue.Format   = DISPLAY_OUTPUT_FORMAT;
		m_ColorClearValue.Color[0] = 0.0f;
		m_ColorClearValue.Color[1] = 0.0f;
		m_ColorClearValue.Color[2] = 0.0f;
		m_ColorClearValue.Color[3] = 0.0f;

		m_DepthClearValue.Format   = DEPTH_FORMAT;
		m_DepthClearValue.DepthStencil = { 0, 0 };
	}

	EditorPrimitiveRenderBuffer::~EditorPrimitiveRenderBuffer()
	{
		if (m_ColorTarget) { m_ColorTarget->ReleaseBufferedResource(); }
		if (m_DepthTarget) { m_DepthTarget->ReleaseBufferedResource(); }
	}

	void EditorPrimitiveRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_DESCRIPTOR_HEAP_DESC ColorHeapDesc = {};
		ColorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		ColorHeapDesc.NumDescriptors = 1;
		Device->CreateDescriptorHeap( &ColorHeapDesc, IID_PPV_ARGS(m_ColorRtvHeap.ReleaseAndGetAddressOf()) );
		m_ColorCpuHandle = m_ColorRtvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = 1;
		Device->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DepthSrvHeap.ReleaseAndGetAddressOf()) );
		m_DepthCpuHandle = m_DepthSrvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void EditorPrimitiveRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		if (m_ColorTarget)
			m_ColorTarget->ReleaseBufferedResource();

		m_ColorTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DISPLAY_OUTPUT_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
			D3D12_RESOURCE_STATE_RENDER_TARGET, m_ColorClearValue);

		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
		RenderTargetViewDesc.Format = DISPLAY_OUTPUT_FORMAT;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;

		Device->CreateRenderTargetView( m_ColorTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_ColorCpuHandle );

		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Format = DISPLAY_OUTPUT_FORMAT;
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SrvDesc.Texture2D.MipLevels = 1;
		SrvDesc.Texture2D.MostDetailedMip = 0;

		Device->CreateShaderResourceView( m_ColorTarget->GetD3D12Resource(), &SrvDesc, m_ColorTarget->GetCpuHandle() );

		if (m_DepthTarget)
			m_DepthTarget->ReleaseBufferedResource();
		
		m_DepthTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DEPTH_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, m_DepthClearValue);
		
		D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
		DepthViewDesc.Format = DEPTH_FORMAT;
		DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DepthViewDesc.Texture2D.MipSlice = 0;
		
		Device->CreateDepthStencilView( m_DepthTarget->GetD3D12Resource(), &DepthViewDesc, m_DepthCpuHandle );

#if D3D12_Debug_INFO
		m_ColorTarget->SetName( "EditorPrimitiveColor" );
		m_DepthTarget->SetName( "EditorPrimitiveDepth" );
#endif
	}

	void EditorPrimitiveRenderBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->ClearRenderTargetView( m_ColorCpuHandle, m_ColorClearValue.Color, 0, nullptr );
		CommandList->ClearDepthStencilView( m_DepthCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0, 0, 0, nullptr );
	}

	void EditorPrimitiveRenderBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);

		CommandList->OMSetRenderTargets( 1, &m_ColorCpuHandle, true, &m_DepthCpuHandle );
	}

}