#include "DrnPCH.h"
#include "TAABuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"

namespace Drn
{
	float TAABuffer::m_JitterOffsets[4][2] = 
	{
		{-0.5f, -0.5f},
		{0.5f, -0.5f},
		{0.5f, 0.5f},
		{-0.5f, 0.5f}
	};

	TAABuffer::TAABuffer()
		: RenderBuffer()
		, m_TAATarget(nullptr)
		, m_Buffer(nullptr)
	{
		
	}

	TAABuffer::~TAABuffer()
	{
		ReleaseBuffers();

		Renderer::Get()->m_BindlessSrvHeapAllocator.Free(m_CpuHandle, m_GpuHandle);
	}

	void TAABuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		Renderer::Get()->m_BindlessSrvHeapAllocator.Alloc( &m_CpuHandle, &m_GpuHandle );

// ---------------------------------------------------------------------------------------------------------------

		m_Buffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_Buffer->SetName("CB_TAA");
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_Buffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_Buffer->GetCpuHandle());
	}

	void TAABuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		if (m_TAATarget)
		{
			m_TAATarget->ReleaseBufferedResource();
			m_TAATarget = nullptr;
		}

		m_TAATarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_COLOR_DEFERRED_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_COMMON);

		D3D12_UNORDERED_ACCESS_VIEW_DESC DescUAV = {};
		DescUAV.Format = GBUFFER_COLOR_DEFERRED_FORMAT;
		DescUAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		DescUAV.Texture2D.MipSlice = 0;
		DescUAV.Texture2D.PlaneSlice = 0;
		
		Device->CreateUnorderedAccessView(m_TAATarget->GetD3D12Resource(), nullptr, &DescUAV, m_CpuHandle);

#if D3D12_Debug_INFO
		m_TAATarget->SetName( "TAATarget" );
#endif

// --------------------------------------------------------------------------------------------------------------

		D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
		Desc.Format = GBUFFER_COLOR_DEFERRED_FORMAT;
		Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		Desc.Texture2D.MipLevels = 1;
		Desc.Texture2D.MostDetailedMip = 0;

		Device->CreateShaderResourceView(m_TAATarget->GetD3D12Resource(), &Desc, m_TAATarget->GetCpuHandle());
	}

	void TAABuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void TAABuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
	}

	void TAABuffer::MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		m_Data.DeferredColorTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_ColorDeferredTarget->GetGpuHandle());
		m_Data.HistoryTexture = Renderer::Get()->GetBindlessSrvIndex(m_GpuHandle);

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_Buffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_Data, sizeof(TAABuffer));
		m_Buffer->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void TAABuffer::ReleaseBuffers()
	{
		if (m_TAATarget)
		{
			m_TAATarget->ReleaseBufferedResource();
			m_TAATarget = nullptr;
		}

		if (m_Buffer)
		{
			m_Buffer->ReleaseBufferedResource();
			m_Buffer = nullptr;
		}
	}

	
}