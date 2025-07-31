#include "DrnPCH.h"
#include "EditorSelectionRenderBuffer.h"

namespace Drn
{
	EditorSelectionRenderBuffer::EditorSelectionRenderBuffer()
		: RenderBuffer()
		, m_DepthStencilTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		m_DepthStencilClearValue.Format   = DEPTH_STENCIL_FORMAT;
		m_DepthStencilClearValue.DepthStencil = { 0, 0 };
	}

	EditorSelectionRenderBuffer::~EditorSelectionRenderBuffer()
	{
		if (m_DepthStencilTarget) { m_DepthStencilTarget->ReleaseBufferedResource(); }

	}

	void EditorSelectionRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc = {};
		DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DsvHeapDesc.NumDescriptors = 1;
		Device->CreateDescriptorHeap( &DsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );
		m_DepthStencilCpuHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();

#if D3D12_Debug_INFO
		m_DsvHeap->SetName(L"DsvHeapÜEditorSelection");
#endif
	}

	void EditorSelectionRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		if (m_DepthStencilTarget)
			m_DepthStencilTarget->ReleaseBufferedResource();
		
		m_DepthStencilTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DEPTH_STENCIL_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, m_DepthStencilClearValue);
		
		D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
		DepthViewDesc.Format = DEPTH_STENCIL_FORMAT;
		DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DepthViewDesc.Texture2D.MipSlice = 0;
		
		Device->CreateDepthStencilView( m_DepthStencilTarget->GetD3D12Resource(), &DepthViewDesc, m_DepthStencilCpuHandle );

#if D3D12_Debug_INFO
		m_DepthStencilTarget->SetName( "EditorPrimitiveColor" );
#endif

		D3D12_SHADER_RESOURCE_VIEW_DESC DsvSelectionDesc = {};
		//DsvSelectionDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		DsvSelectionDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		DsvSelectionDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		DsvSelectionDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		DsvSelectionDesc.Texture2D.PlaneSlice = 1;
		DsvSelectionDesc.Texture2D.MipLevels = 1;
		DsvSelectionDesc.Texture2D.MostDetailedMip = 0;
		
		Device->CreateShaderResourceView( m_DepthStencilTarget->GetD3D12Resource(), &DsvSelectionDesc, m_DepthStencilTarget->GetCpuHandle() );

		// use this as another srv if needed depth
		//D3D12_SHADER_RESOURCE_VIEW_DESC DsvSelectionDesc = {};
		//DsvSelectionDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		//DsvSelectionDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//DsvSelectionDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//DsvSelectionDesc.Texture2D.PlaneSlice = 1;
		//DsvSelectionDesc.Texture2D.MipLevels = 1;
		//DsvSelectionDesc.Texture2D.MostDetailedMip = 0;
	}

	void EditorSelectionRenderBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->ClearDepthStencilView( m_DepthStencilCpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL , 0, 0, 0, nullptr );
	}

	void EditorSelectionRenderBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);

		CommandList->OMSetRenderTargets( 0, nullptr, true, &m_DepthStencilCpuHandle );
	}

}