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
		, m_DepthTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		m_ColorDeferredClearValue.Format   = GBUFFER_COLOR_DEFERRED_FORMAT;
		m_ColorDeferredClearValue.Color[0] = 0.04f;
		m_ColorDeferredClearValue.Color[1] = 0.07f;
		m_ColorDeferredClearValue.Color[2] = 0.2f;
		m_ColorDeferredClearValue.Color[3] = 1.0f;

		m_BaseColorClearValue.Format   = GBUFFER_BASE_COLOR_FORMAT;
		m_BaseColorClearValue.Color[0] = 0.0f;
		m_BaseColorClearValue.Color[1] = 0.0f;
		m_BaseColorClearValue.Color[2] = 0.0f;
		m_BaseColorClearValue.Color[3] = 1.0f;

		m_WorldNormalClearValue.Format   = GBUFFER_WORLD_NORMAL_FORMAT;
		m_WorldNormalClearValue.Color[0] = 0.0f;
		m_WorldNormalClearValue.Color[1] = 0.0f;
		m_WorldNormalClearValue.Color[2] = 0.0f;
		m_WorldNormalClearValue.Color[3] = 1.0f;
		
		m_DepthClearValue.Format = DEPTH_FORMAT;
		m_DepthClearValue.DepthStencil = { 0, 0 };

		Renderer::Get()->TempSRVAllocator.Alloc(&m_ColorDeferredSrvCpuHandle, &m_ColorDeferredSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Alloc(&m_BaseColorSrvCpuHandle, &m_BaseColorSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Alloc(&m_WorldNormalSrvCpuHandle, &m_WorldNormalSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Alloc(&m_MasksSrvCpuHandle, &m_MasksSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Alloc(&m_DepthSrvCpuHandle, &m_DepthSrvGpuHandle);
	}

	GBuffer::~GBuffer()
	{
		if (m_ColorDeferredTarget) { m_ColorDeferredTarget->ReleaseBufferedResource(); }
		if (m_BaseColorTarget) { m_BaseColorTarget->ReleaseBufferedResource(); }
		if (m_WorldNormalTarget) { m_WorldNormalTarget->ReleaseBufferedResource(); }
		if (m_MasksTarget) { m_MasksTarget->ReleaseBufferedResource(); }
		if (m_DepthTarget) { m_DepthTarget->ReleaseBufferedResource(); }

		Renderer::Get()->TempSRVAllocator.Free(m_ColorDeferredSrvCpuHandle, m_ColorDeferredSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Free(m_BaseColorSrvCpuHandle, m_BaseColorSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Free(m_WorldNormalSrvCpuHandle, m_WorldNormalSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Free(m_MasksSrvCpuHandle, m_MasksSrvGpuHandle);
		Renderer::Get()->TempSRVAllocator.Free(m_DepthSrvCpuHandle, m_DepthSrvGpuHandle);
	}

	void GBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_DESCRIPTOR_HEAP_DESC GuidHeapDesc = {};
		GuidHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		GuidHeapDesc.NumDescriptors = 4;
		Device->CreateDescriptorHeap( &GuidHeapDesc, IID_PPV_ARGS(m_RtvHeap.ReleaseAndGetAddressOf()) );

		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = 1;
		Device->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );

		m_ColorDeferredCpuHandle	= CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), 0, Renderer::Get()->GetRtvIncrementSize());
		m_BaseColorCpuHandle		= CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), 1, Renderer::Get()->GetRtvIncrementSize());
		m_WorldNormalCpuHandle		= CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, Renderer::Get()->GetRtvIncrementSize());
		m_MasksCpuHandle			= CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, Renderer::Get()->GetRtvIncrementSize());
		m_DepthCpuHandle			= m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void GBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		{
			if (m_ColorDeferredTarget)
				m_ColorDeferredTarget->ReleaseBufferedResource();

			m_ColorDeferredTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_COLOR_DEFERRED_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET, m_ColorDeferredClearValue);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = GBUFFER_COLOR_DEFERRED_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_ColorDeferredTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_ColorDeferredCpuHandle );

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = GBUFFER_COLOR_DEFERRED_FORMAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView( m_ColorDeferredTarget->GetD3D12Resource(), &SrvDesc, m_ColorDeferredSrvCpuHandle );
		}

		{
			if (m_BaseColorTarget)
				m_BaseColorTarget->ReleaseBufferedResource();

			m_BaseColorTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_BASE_COLOR_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET, m_BaseColorClearValue);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = GBUFFER_BASE_COLOR_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_BaseColorTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_BaseColorCpuHandle );

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = GBUFFER_BASE_COLOR_FORMAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView( m_BaseColorTarget->GetD3D12Resource(), &SrvDesc, m_BaseColorSrvCpuHandle );
		}

		{
			if (m_WorldNormalTarget)
				m_WorldNormalTarget->ReleaseBufferedResource();

			m_WorldNormalTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_WORLD_NORMAL_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET, m_WorldNormalClearValue);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = GBUFFER_WORLD_NORMAL_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_WorldNormalTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_WorldNormalCpuHandle );

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = GBUFFER_WORLD_NORMAL_FORMAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView( m_WorldNormalTarget->GetD3D12Resource(), &SrvDesc, m_WorldNormalSrvCpuHandle );
		}

		{
			if (m_MasksTarget)
				m_MasksTarget->ReleaseBufferedResource();

			m_MasksTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_MASKS_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET, m_BaseColorClearValue);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = GBUFFER_MASKS_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_MasksTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_MasksCpuHandle );

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = GBUFFER_MASKS_FORMAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView( m_MasksTarget->GetD3D12Resource(), &SrvDesc, m_MasksSrvCpuHandle );
		}

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

			Device->CreateShaderResourceView( m_DepthTarget->GetD3D12Resource(), &SrvDesc, m_DepthSrvCpuHandle );
		}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if D3D12_Debug_INFO
		m_ColorDeferredTarget->SetName( "Gbuffer_DeferredColor" );
		m_BaseColorTarget->SetName( "Gbuffer_BaseColor" );
		m_WorldNormalTarget->SetName( "Gbuffer_WorldNormal" );
		m_MasksTarget->SetName( "Gbuffer_Masks" );
		m_DepthTarget->SetName( "Gbuffer_Depth" );
#endif

	}

	void GBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->ClearRenderTargetView( m_ColorDeferredCpuHandle, m_ColorDeferredClearValue.Color, 0, nullptr );
		CommandList->ClearRenderTargetView( m_BaseColorCpuHandle, m_BaseColorClearValue.Color, 0, nullptr );
		CommandList->ClearRenderTargetView( m_WorldNormalCpuHandle, m_WorldNormalClearValue.Color, 0, nullptr );
		CommandList->ClearRenderTargetView( m_MasksCpuHandle, m_BaseColorClearValue.Color, 0, nullptr );
		CommandList->ClearDepthStencilView( m_DepthCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0, 0, 0, nullptr );
	}

	void GBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE const RenderTargets[4] = 
		{
			m_ColorDeferredCpuHandle,
			m_BaseColorCpuHandle,
			m_WorldNormalCpuHandle,
			m_MasksCpuHandle,
		};

		CommandList->OMSetRenderTargets( 4, RenderTargets, false, &m_DepthCpuHandle );
	}

	void GBuffer::BindLightPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CD3DX12_RESOURCE_BARRIER const Barriers[4] = 
		{
			CD3DX12_RESOURCE_BARRIER::Transition( m_BaseColorTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE ),
			CD3DX12_RESOURCE_BARRIER::Transition( m_WorldNormalTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE ),
			CD3DX12_RESOURCE_BARRIER::Transition( m_MasksTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE ),
			CD3DX12_RESOURCE_BARRIER::Transition( m_DepthTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE )
		};

		CommandList->ResourceBarrier(4, Barriers);

		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);

		CommandList->OMSetRenderTargets( 1, &m_ColorDeferredCpuHandle, true, nullptr );

		CommandList->SetGraphicsRootDescriptorTable(1, m_BaseColorSrvGpuHandle);
		CommandList->SetGraphicsRootDescriptorTable(2, m_WorldNormalSrvGpuHandle);
		CommandList->SetGraphicsRootDescriptorTable(3, m_MasksSrvGpuHandle);
		CommandList->SetGraphicsRootDescriptorTable(4, m_DepthSrvGpuHandle);
	}

	void GBuffer::UnBindLightPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CD3DX12_RESOURCE_BARRIER const Barriers[4] = 
		{
			CD3DX12_RESOURCE_BARRIER::Transition( m_BaseColorTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET ),
			CD3DX12_RESOURCE_BARRIER::Transition( m_WorldNormalTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET ),
			CD3DX12_RESOURCE_BARRIER::Transition( m_MasksTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET ),
			CD3DX12_RESOURCE_BARRIER::Transition( m_DepthTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE )
		};

		CommandList->ResourceBarrier(4, Barriers);
	}

}