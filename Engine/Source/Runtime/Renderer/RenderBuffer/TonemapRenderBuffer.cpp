#include "DrnPCH.h"
#include "TonemapRenderBuffer.h"

namespace Drn
{
	TonemapRenderBuffer::TonemapRenderBuffer()
		: RenderBuffer()
		, m_TonemapTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{}

	TonemapRenderBuffer::~TonemapRenderBuffer()
	{}

	void TonemapRenderBuffer::Init()
	{
		RenderBuffer::Init();
	}

	void TonemapRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		RenderResourceCreateInfo TonemapTargetCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "TonemapTarget" );
		m_TonemapTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DISPLAY_OUTPUT_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), TonemapTargetCreateInfo);
	}

	void TonemapRenderBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_TonemapTarget);
	}

	void TonemapRenderBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->GetD3D12CommandList()->RSSetViewports(1, &m_Viewport);
		CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE TonemapHandle = m_TonemapTarget->GetRenderTargetView()->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &TonemapHandle, true, nullptr );
	}

}