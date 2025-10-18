#include "DrnPCH.h"
#include "DecalBuffer.h"

namespace Drn
{
	DecalBuffer::DecalBuffer()
		: RenderBuffer()
		, m_BaseColorTarget(nullptr)
		, m_NormalTarget(nullptr)
		, m_MasksTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		m_BaseColorClearValue.Format   = DECAL_BASE_COLOR_FORMAT;
		m_BaseColorClearValue.Color[0] = 0.0f;
		m_BaseColorClearValue.Color[1] = 0.0f;
		m_BaseColorClearValue.Color[2] = 0.0f;
		m_BaseColorClearValue.Color[3] = 1.0f;

		m_NormalClearValue.Format   = DECAL_NORMAL_FORMAT;
		m_NormalClearValue.Color[0] = 0.5f;
		m_NormalClearValue.Color[1] = 0.5f;
		m_NormalClearValue.Color[2] = 1.0f;
		m_NormalClearValue.Color[3] = 1.0f;
		
		m_MasksClearValue.Format   = DECAL_MASKS_FORMAT;
		m_MasksClearValue.Color[0] = 0.0f;
		m_MasksClearValue.Color[1] = 0.0f;
		m_MasksClearValue.Color[2] = 0.0f;
		m_MasksClearValue.Color[3] = 1.0f;
	}

	DecalBuffer::~DecalBuffer()
	{
		ReleaseBuffers();
		ReleaseHandles();
	}

	void DecalBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		AllocateHandles();
	}

	void DecalBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		ReleaseBuffers();

		{
			m_BaseColorTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(DECAL_BASE_COLOR_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET, m_BaseColorClearValue);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = DECAL_BASE_COLOR_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			m_BaseColorRTVHandle.CreateView(RenderTargetViewDesc, m_BaseColorTarget->GetD3D12Resource());

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = DECAL_BASE_COLOR_FORMAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			m_BaseColorSRVHandle.CreateView(SrvDesc, m_BaseColorTarget->GetD3D12Resource());
		}

		{
			m_NormalTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(DECAL_NORMAL_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET, m_NormalClearValue);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = DECAL_NORMAL_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			m_NormalRTVHandle.CreateView(RenderTargetViewDesc, m_NormalTarget->GetD3D12Resource());

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = DECAL_NORMAL_FORMAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			m_NormalSRVHandle.CreateView(SrvDesc, m_NormalTarget->GetD3D12Resource());
		}

		{
			m_MasksTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(DECAL_MASKS_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET, m_MasksClearValue);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = DECAL_MASKS_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			m_MasksRTVHandle.CreateView(RenderTargetViewDesc, m_MasksTarget->GetD3D12Resource());

			D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
			SrvDesc.Format = DECAL_MASKS_FORMAT;
			SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = 1;
			SrvDesc.Texture2D.MostDetailedMip = 0;

			m_MasksSRVHandle.CreateView(SrvDesc, m_MasksTarget->GetD3D12Resource());
		}

#if D3D12_Debug_INFO
		m_BaseColorTarget->SetName( "Decal_BaseColor" );
		m_NormalTarget->SetName( "Decal_Normal" );
		m_MasksTarget->SetName( "Decal_Masks" );
#endif
	}

	void DecalBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->ClearRenderTargetView( m_BaseColorRTVHandle.GetCpuHandle(), m_BaseColorClearValue.Color, 0, nullptr );
		CommandList->ClearRenderTargetView( m_NormalRTVHandle.GetCpuHandle(), m_NormalClearValue.Color, 0, nullptr );
		CommandList->ClearRenderTargetView( m_MasksRTVHandle.GetCpuHandle(), m_MasksClearValue.Color, 0, nullptr );
	}

	void DecalBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE const RenderTargets[3] = 
		{
			m_BaseColorRTVHandle.GetCpuHandle(),
			m_NormalRTVHandle.GetCpuHandle(),
			m_MasksRTVHandle.GetCpuHandle()
		};

		CommandList->OMSetRenderTargets( 3, RenderTargets, false, NULL );
	}

	void DecalBuffer::ReleaseBuffers()
	{
		if (m_BaseColorTarget) { m_BaseColorTarget->ReleaseBufferedResource(); }
		if (m_NormalTarget) { m_NormalTarget->ReleaseBufferedResource(); }
		if (m_MasksTarget) { m_MasksTarget->ReleaseBufferedResource(); }
	}

	void DecalBuffer::AllocateHandles()
	{
		m_BaseColorRTVHandle.AllocateDescriptorSlot();
		m_BaseColorSRVHandle.AllocateDescriptorSlot();

		m_NormalRTVHandle.AllocateDescriptorSlot();
		m_NormalSRVHandle.AllocateDescriptorSlot();

		m_MasksRTVHandle.AllocateDescriptorSlot();
		m_MasksSRVHandle.AllocateDescriptorSlot();
	}

	void DecalBuffer::ReleaseHandles()
	{
		m_BaseColorRTVHandle.FreeDescriptorSlot();
		m_BaseColorSRVHandle.FreeDescriptorSlot();

		m_NormalRTVHandle.FreeDescriptorSlot();
		m_NormalSRVHandle.FreeDescriptorSlot();

		m_MasksRTVHandle.FreeDescriptorSlot();
		m_MasksSRVHandle.FreeDescriptorSlot();
	}

}