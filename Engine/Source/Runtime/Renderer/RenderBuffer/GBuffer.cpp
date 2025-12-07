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
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		m_DepthClearValue.Format = DEPTH_FORMAT;
		m_DepthClearValue.DepthStencil = { 0, 0 };
	}

	GBuffer::~GBuffer()
	{
		if (m_DepthTarget) { m_DepthTarget->ReleaseBufferedResource(); }
	}

	void GBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_DESCRIPTOR_HEAP_DESC GuidHeapDesc = {};
		GuidHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		GuidHeapDesc.NumDescriptors = 6;
		Device->CreateDescriptorHeap( &GuidHeapDesc, IID_PPV_ARGS(m_RtvHeap.ReleaseAndGetAddressOf()) );

		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = 1;
		Device->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );

		m_DepthCpuHandle			= m_DsvHeap->GetCPUDescriptorHandleForHeapStart();

#if D3D12_Debug_INFO
		m_RtvHeap->SetName(L"RtvHeap_Gbuffer");
		m_DsvHeap->SetName(L"DsvHeap_Gbuffer");
#endif
	}

	void GBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

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

		{
			if (m_DepthTarget)
				m_DepthTarget->ReleaseBufferedResource();
		
			m_DepthTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(DEPTH_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
				D3D12_RESOURCE_STATE_DEPTH_WRITE, m_DepthClearValue);
		
			D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
			DepthViewDesc.Format = DEPTH_FORMAT;
			DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			DepthViewDesc.Texture2D.MipSlice = 0;
		
			Device->CreateDepthStencilView( m_DepthTarget->GetD3D12Resource(), &DepthViewDesc, m_DsvHeap->GetCPUDescriptorHandleForHeapStart() );

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView( m_DepthTarget->GetD3D12Resource(), &SrvDesc, m_DepthTarget->GetCpuHandle() );
		}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if D3D12_Debug_INFO
		m_DepthTarget->SetName( "Gbuffer_Depth" );
#endif

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
		CommandList->GetD3D12CommandList()->ClearDepthStencilView( m_DepthCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0, 0, 0, nullptr );
	}

	void GBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->GetD3D12CommandList()->RSSetViewports(1, &m_Viewport);
		CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE const RenderTargets[6] = 
		{
			m_ColorDeferredTarget->GetRenderTargetView( 0, 0 )->GetView(),
			m_BaseColorTarget->GetRenderTargetView( 0, 0 )->GetView(),
			m_WorldNormalTarget->GetRenderTargetView( 0, 0 )->GetView(),
			m_MasksTarget->GetRenderTargetView(0, 0)->GetView(),
			m_MasksBTarget->GetRenderTargetView(0, 0)->GetView(),
			m_VelocityTarget->GetRenderTargetView(0, 0)->GetView(),
		};

		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 6, RenderTargets, false, &m_DepthCpuHandle );
	}

	void GBuffer::BindDepth( D3D12CommandList* CommandList )
	{
		CommandList->GetD3D12CommandList()->RSSetViewports(1, &m_Viewport);
		CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &m_ScissorRect);

		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 0, NULL, true, &m_DepthCpuHandle );
	}

	void GBuffer::BindLightPass( D3D12CommandList* CommandList )
	{
		ResourceStateTracker::Get()->TransiationResource(m_DepthTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->FlushResourceBarriers(CommandList->GetD3D12CommandList());

		CommandList->TransitionResourceWithTracking(m_BaseColorTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->TransitionResourceWithTracking(m_WorldNormalTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->TransitionResourceWithTracking(m_MasksTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->TransitionResourceWithTracking(m_MasksBTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CommandList->FlushBarriers();


		CommandList->GetD3D12CommandList()->RSSetViewports(1, &m_Viewport);
		CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE DeferredColorHandle = m_ColorDeferredTarget->GetRenderTargetView( 0, 0 )->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &DeferredColorHandle, true, nullptr );
	}

}