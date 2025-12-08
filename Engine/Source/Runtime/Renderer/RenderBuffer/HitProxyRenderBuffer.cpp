#include "DrnPCH.h"
#include "HitProxyRenderBuffer.h"

#if WITH_EDITOR

namespace Drn
{
	HitProxyRenderBuffer::HitProxyRenderBuffer()
		: RenderBuffer()
		, m_GuidTarget(nullptr)
		, m_DepthTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{}

	HitProxyRenderBuffer::~HitProxyRenderBuffer()
	{}

	void HitProxyRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void HitProxyRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		RenderResourceCreateInfo GuidCreateInfo( nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "HitProxy_GuidTarget" );
		m_GuidTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_GUID_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), GuidCreateInfo);

		RenderResourceCreateInfo DepthCreateInfo( nullptr, nullptr, ClearValueBinding::DepthZero, "HitProxy_DepthTarget" );
		m_DepthTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DEPTH_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), DepthCreateInfo);
	}

	void HitProxyRenderBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_GuidTarget);
		CommandList->ClearDepthTexture(m_DepthTarget, EDepthStencilViewType::DepthStencilWrite, true, false);
	}

	void HitProxyRenderBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->GetD3D12CommandList()->RSSetViewports(1, &m_Viewport);
		CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE GuidHandle = m_GuidTarget->GetRenderTargetView()->GetView();
		D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle = m_DepthTarget->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &GuidHandle, 1, &DepthHandle);
	}
}
#endif