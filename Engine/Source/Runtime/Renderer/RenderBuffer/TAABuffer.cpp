#include "DrnPCH.h"
#include "TAABuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"

namespace Drn
{
	float TAABuffer::m_JitterOffsets[8][2] = 
	{
		{0.0, -0.33334},
		{-0.5, 0.333334},
		{0.5f, -0.777778},
		{-0.75, -0.111112},
		{0.25, 0.555556},
		{-0.25, -0.555556},
		{ 0.75, 0.111112},
		{-0.875, 0.777778},
	};

	//float TAABuffer::m_JitterOffsets[4][2] = 
	//{
	//	{-0.5f, -0.5f},
	//	{0.5f, -0.5f},
	//	{0.5f, 0.5f},
	//	{-0.5f, 0.5f}
	//};

	//float TAABuffer::m_JitterOffsets[2][2] = 
	//{
	//	{0.5f, 0.0f},
	//	{0.0f, 0.5f}
	//};

	TAABuffer::TAABuffer()
		: RenderBuffer()
	{
		
	}

	TAABuffer::~TAABuffer()
	{
		ReleaseBuffers();

		for ( int32 i = 0; i < 2; i++)
		{
			m_UAVHandles[i].FreeDescriptorSlot();
			m_SrvHandles[i].FreeDescriptorSlot();
		}
	}

	void TAABuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < 2; i++)
		{
			m_UAVHandles[i].AllocateDescriptorSlot();
			m_SrvHandles[i].AllocateDescriptorSlot();
		}

// ---------------------------------------------------------------------------------------------------------------

		for (int32 i = 0; i < NUM_BACKBUFFERS; i++)
		{
			m_Buffer[i] = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
	#if D3D12_Debug_INFO
			m_Buffer[i]->SetName("CB_TAA");
	#endif

			D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
			ResourceViewDesc.BufferLocation = m_Buffer[i]->GetD3D12Resource()->GetGPUVirtualAddress();
			ResourceViewDesc.SizeInBytes = 256;
			Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_Buffer[i]->GetCpuHandle());
		}

	}

	void TAABuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < 2; i++)
		{
			if ( m_TAATarget[i] )
			{
				m_TAATarget[i]->ReleaseBufferedResource();
				m_TAATarget[i] = nullptr;
			}


			m_TAATarget[i] = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_COLOR_DEFERRED_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
				D3D12_RESOURCE_STATE_COMMON);

			D3D12_UNORDERED_ACCESS_VIEW_DESC DescUAV = {};
			DescUAV.Format = GBUFFER_COLOR_DEFERRED_FORMAT;
			DescUAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			DescUAV.Texture2D.MipSlice = 0;
			DescUAV.Texture2D.PlaneSlice = 0;
		
			m_UAVHandles[i].CreateUnorderedView( DescUAV, m_TAATarget[i]->GetD3D12Resource() );

#if D3D12_Debug_INFO
			m_TAATarget[i]->SetName( "TAATarget_" + std::to_string(i) );
#endif
// --------------------------------------------------------------------------------------------------------------

			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = GBUFFER_COLOR_DEFERRED_FORMAT;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = 0;

			m_SrvHandles[i].CreateView(Desc, m_TAATarget[i]->GetD3D12Resource());
		}

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
		m_Data.VelocityTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_VelocityTarget->GetGpuHandle());
		m_Data.HistoryTexture = GetHistorySRV(Renderer->m_SceneView.FrameIndex).GetIndex();
		m_Data.TargetTexture = GetFrameUAV(Renderer->m_SceneView.FrameIndex).GetIndex();

		m_Data.DepthTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_DepthTarget->GetGpuHandle());
		m_Data.CurrentFrameWeight = Renderer->m_PostProcessSettings->m_TAASettings.m_CurrentFrameWeight;
		m_Data.CcurrentFrameVelocityWeight = Renderer->m_PostProcessSettings->m_TAASettings.m_CcurrentFrameVelocityWeight;
		m_Data.CcurrentFrameVelocityMultiplier = Renderer->m_PostProcessSettings->m_TAASettings.m_CcurrentFrameVelocityMultiplier;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_Buffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_Data, sizeof(TAABuffer));
		m_Buffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void TAABuffer::ReleaseBuffers()
	{
		for (int32 i = 0; i < 2; i++)
		{
			if (m_TAATarget[i])
			{
				m_TAATarget[i]->ReleaseBufferedResource();
				m_TAATarget[i] = nullptr;
			}
		}

		for (int32 i = 0; i < NUM_BACKBUFFERS; i++)
		{
			if (m_Buffer[i])
			{
				m_Buffer[i]->ReleaseBufferedResource();
				m_Buffer[i] = nullptr;
			}
		}
	}

	
}