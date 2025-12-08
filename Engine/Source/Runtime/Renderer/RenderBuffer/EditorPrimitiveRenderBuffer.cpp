#include "DrnPCH.h"
#include "EditorPrimitiveRenderBuffer.h"

namespace Drn
{
	EditorPrimitiveRenderBuffer::EditorPrimitiveRenderBuffer()
		: RenderBuffer()
		, m_ColorTarget(nullptr)
		, m_DepthTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{}

	EditorPrimitiveRenderBuffer::~EditorPrimitiveRenderBuffer()
	{}

	void EditorPrimitiveRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void EditorPrimitiveRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		RenderResourceCreateInfo ColorCreateInfo( nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "EditorPrimitiveColor" );
		m_ColorTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DISPLAY_OUTPUT_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), ColorCreateInfo);

		RenderResourceCreateInfo DepthCreateInfo( nullptr, nullptr, ClearValueBinding::DepthZero, "EditorPrimitiveDepth" );
		m_DepthTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DEPTH_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), DepthCreateInfo);
	}

	void EditorPrimitiveRenderBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_ColorTarget);
		//CommandList->ClearDepthStencilView( m_DepthCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0, 0, 0, nullptr );
	}

	void EditorPrimitiveRenderBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->GetD3D12CommandList()->RSSetViewports(1, &m_Viewport);
		CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE ColorHandle = m_ColorTarget->GetRenderTargetView()->GetView();
		D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle = m_DepthTarget->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &ColorHandle, true, &DepthHandle );
	}

}