#include "DrnPCH.h"
#include "ScreenSpaceReflectionBuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/HZBBuffer.h"

#define SSR_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT

namespace Drn
{
	ScreenSpaceReflectionBuffer::ScreenSpaceReflectionBuffer()
		: RenderBuffer()
		, m_Target(nullptr)
		, m_Buffer(nullptr)
	{
		m_ClearValue.Format   = SSR_FORMAT;
		m_ClearValue.Color[0] = 0.0f;
		m_ClearValue.Color[1] = 0.0f;
		m_ClearValue.Color[2] = 0.0f;
		m_ClearValue.Color[3] = 0.0f;
	}

	ScreenSpaceReflectionBuffer::~ScreenSpaceReflectionBuffer()
	{
		ReleaseBuffers();
	}

	void ScreenSpaceReflectionBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		HeapDesc.NumDescriptors = 1;
		Device->CreateDescriptorHeap( &HeapDesc, IID_PPV_ARGS(m_RtvHeap.ReleaseAndGetAddressOf()) );

		m_Handle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
#if D3D12_Debug_INFO
		m_RtvHeap->SetName(L"RtvHeap_SSR");
#endif

// ---------------------------------------------------------------------------------------------------------------

		m_Buffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_Buffer->SetName("CB_SSR");
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_Buffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_Buffer->GetCpuHandle());
	}

	void ScreenSpaceReflectionBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		if (m_Target)
		{
			m_Target->ReleaseBufferedResource();
			m_Target = nullptr;
		}

		m_Target = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(SSR_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
			D3D12_RESOURCE_STATE_RENDER_TARGET, m_ClearValue);

		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
		RenderTargetViewDesc.Format = SSR_FORMAT;
		RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;

		Device->CreateRenderTargetView( m_Target->GetD3D12Resource(), &RenderTargetViewDesc, m_RtvHeap->GetCPUDescriptorHandleForHeapStart() );

#if D3D12_Debug_INFO
		m_Target->SetName( "SSRTarget" );
#endif

// --------------------------------------------------------------------------------------------------------------

		D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
		Desc.Format = SSR_FORMAT;
		Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		Desc.Texture2D.MipLevels = 1;
		Desc.Texture2D.MostDetailedMip = 0;

		Device->CreateShaderResourceView(m_Target->GetD3D12Resource(), &Desc, m_Target->GetCpuHandle());
	}

	void ScreenSpaceReflectionBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->ClearRenderTargetView( m_Handle, m_ClearValue.Color, 0, nullptr );
	}

	void ScreenSpaceReflectionBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
		D3D12_VIEWPORT Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_Size.X), static_cast<float>(m_Size.Y));

		CommandList->RSSetViewports(1, &Viewport);
		CommandList->RSSetScissorRects(1, &ScissorRect);

		CommandList->OMSetRenderTargets( 1, &m_Handle, true, nullptr );
	}

	void ScreenSpaceReflectionBuffer::MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer, const SSRSettings& Settings )
	{
		m_Data.DeferredColorTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_ColorDeferredTarget->GetGpuHandle());
		m_Data.BaseColorTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_BaseColorTarget->GetGpuHandle());
		m_Data.WorldNormalTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_WorldNormalTarget->GetGpuHandle());
		m_Data.MasksTexture = Renderer->m_GBuffer->m_MasksTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.DepthTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_DepthTarget->GetGpuHandle());
		m_Data.HzbTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_HZBBuffer->M_HZBTarget->GetGpuHandle());

		m_Data.Intensity = Settings.m_Intensity;
		m_Data.RoughnessFade = Settings.m_RoughnessFade;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_Buffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_Data, sizeof(ScreenSpaceRefletcionData));
		m_Buffer->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void ScreenSpaceReflectionBuffer::ReleaseBuffers()
	{
		if (m_Target)
		{
			m_Target->ReleaseBufferedResource();
			m_Target = nullptr;
		}

		if (m_Buffer)
		{
			m_Buffer->ReleaseBufferedResource();
			m_Buffer = nullptr;
		}
	}

}