#include "DrnPCH.h"
#include "GBuffer.h"

namespace Drn
{
	GBuffer::GBuffer()
		: RenderBuffer()
		, m_ColorDeferredTarget(nullptr)
		, m_BaseColorTarget(nullptr)
		, m_WorldNormalTarget(nullptr)
		, m_MasksTarget(nullptr)
		, m_MasksBTarget(nullptr)
		, m_VelocityTarget(nullptr)
		, m_DepthTarget(nullptr)
	{}

	GBuffer::~GBuffer()
	{}

	void GBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
	}

	void GBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		RenderResourceCreateInfo ColorDeferredCreateInfo( nullptr, nullptr, ClearValueBinding(Vector4(0.04f, 0.07f, 0.2f, 1.0f)), "Gbuffer_DeferredColor" );
		m_ColorDeferredTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_COLOR_DEFERRED_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), ColorDeferredCreateInfo);

		RenderResourceCreateInfo BaseColorCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "Gbuffer_BaseColor" );
		m_BaseColorTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_BASE_COLOR_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), BaseColorCreateInfo);

		RenderResourceCreateInfo WorldNormalCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "Gbuffer_WorldNormal" );
		m_WorldNormalTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_WORLD_NORMAL_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), WorldNormalCreateInfo);

		RenderResourceCreateInfo MaskACreateInfo( nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "Gbuffer_MasksA" );
		m_MasksTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_MASKS_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), MaskACreateInfo);

		RenderResourceCreateInfo MaskBCreateInfo( nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "Gbuffer_MasksB" );
		m_MasksBTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_MASKS_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), MaskBCreateInfo);

		RenderResourceCreateInfo VelocityCreateInfo( nullptr, nullptr, ClearValueBinding(Vector4(0.5f, 0.5f, 0.0f, 0.0f)), "Gbuffer_Velocity" );
		m_VelocityTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_VELOCITY_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), VelocityCreateInfo);

		RenderResourceCreateInfo DepthCreateInfo( nullptr, nullptr, ClearValueBinding::DepthZero, "Gbuffer_Depth" );
		m_DepthTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DEPTH_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), DepthCreateInfo);
	}

	void GBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_ColorDeferredTarget);
		CommandList->ClearColorTexture(m_BaseColorTarget);
		CommandList->ClearColorTexture(m_WorldNormalTarget);
		CommandList->ClearColorTexture(m_MasksTarget);
		CommandList->ClearColorTexture(m_MasksBTarget);
		CommandList->ClearColorTexture(m_VelocityTarget);

		//CommandList->ClearDepthStencilView( m_DepthCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0, 0, 0, nullptr );
	}

	void GBuffer::ClearDepth( D3D12CommandList* CommandList )
	{
		CommandList->ClearDepthTexture(m_DepthTarget, EDepthStencilViewType::DepthWrite, true, false);
	}

	void GBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport(0 ,0, 0, m_Size.X, m_Size.Y, 1);

		D3D12_CPU_DESCRIPTOR_HANDLE const RenderTargets[6] = 
		{
			m_ColorDeferredTarget->GetRenderTargetView( 0, 0 )->GetView(),
			m_BaseColorTarget->GetRenderTargetView( 0, 0 )->GetView(),
			m_WorldNormalTarget->GetRenderTargetView( 0, 0 )->GetView(),
			m_MasksTarget->GetRenderTargetView(0, 0)->GetView(),
			m_MasksBTarget->GetRenderTargetView(0, 0)->GetView(),
			m_VelocityTarget->GetRenderTargetView(0, 0)->GetView(),
		};

		D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle = m_DepthTarget->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 6, RenderTargets, false, &DepthHandle );
	}

	void GBuffer::BindDepth( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport(0 ,0, 0, m_Size.X, m_Size.Y, 1);

		D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle = m_DepthTarget->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 0, NULL, true, &DepthHandle );
	}

	void GBuffer::BindLightPass( D3D12CommandList* CommandList )
	{
		CommandList->TransitionResourceWithTracking(m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->TransitionResourceWithTracking(m_BaseColorTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->TransitionResourceWithTracking(m_WorldNormalTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->TransitionResourceWithTracking(m_MasksTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->TransitionResourceWithTracking(m_MasksBTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->FlushBarriers();

		CommandList->SetViewport( 0, 0, 0, m_Size.X, m_Size.Y, 1 );

		D3D12_CPU_DESCRIPTOR_HANDLE DeferredColorHandle = m_ColorDeferredTarget->GetRenderTargetView( 0, 0 )->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &DeferredColorHandle, true, nullptr );
	}

}