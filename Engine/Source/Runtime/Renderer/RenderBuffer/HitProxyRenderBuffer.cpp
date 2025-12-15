#include "DrnPCH.h"
#include "HitProxyRenderBuffer.h"

#if WITH_EDITOR

namespace Drn
{
	HitProxyRenderBuffer::HitProxyRenderBuffer()
		: RenderBuffer()
		, m_GuidTarget(nullptr)
		, m_DepthTarget(nullptr)
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
		CommandList->SetViewport(0 ,0, 0, m_Size.X, m_Size.Y, 1);

		D3D12_CPU_DESCRIPTOR_HANDLE GuidHandle = m_GuidTarget->GetRenderTargetView()->GetView();
		D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle = m_DepthTarget->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &GuidHandle, 1, &DepthHandle);
	}
}
#endif