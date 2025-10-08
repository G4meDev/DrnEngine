#include "DrnPCH.h"
#include "BloomBuffer.h"

#include "Runtime/Renderer/RenderBuffer/SceneDownSampleBuffer.h"

namespace Drn
{
	BloomBuffer::BloomBuffer()
	{
		
	}

	BloomBuffer::~BloomBuffer()
	{
		ReleaseBuffers();

		for ( int32 i = 0; i < NUM_BLOOM_TARGETS; i++ )
		{
			m_RTVHandles[i].FreeDescriptorSlot();
			m_SrvHandles[i].FreeDescriptorSlot();
		}
	}

	void BloomBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for ( int32 i = 0; i < NUM_BLOOM_TARGETS; i++ )
		{
			m_RTVHandles[i].AllocateDescriptorSlot();
			m_SrvHandles[i].AllocateDescriptorSlot();
		}

// ---------------------------------------------------------------------------------------------------------------

		for (int32 i = 0; i < NUM_BLOOM_TARGETS; i++)
		{
			m_Buffer[i] = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
			m_Buffer[i]->SetName("CB_Bloom" + GetDownSamplePostfix(i) + ((i%2 == 0) ? "Y" : "X"));
#endif

			D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
			ResourceViewDesc.BufferLocation = m_Buffer[i]->GetD3D12Resource()->GetGPUVirtualAddress();
			ResourceViewDesc.SizeInBytes = 256;
			Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_Buffer[i]->GetCpuHandle());
		}
	}

	void BloomBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < NUM_BLOOM_TARGETS; i++)
		{
			if (m_BloomTargets[i])
			{
				m_BloomTargets[i]->ReleaseBufferedResource();
				m_BloomTargets[i] = nullptr;
			}
		}

		for (int32 i = 0; i < NUM_BLOOM_TARGETS; i++)
		{
			int32 DownSampleIndex = i / 2;
			IntPoint Size = m_Size / std::pow(2, DownSampleIndex + 1);
			Size = IntPoint::ComponentWiseMax(Size, IntPoint(1, 1));

			m_Viewports[DownSampleIndex] = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( Size.X ), static_cast<float>( Size.Y ) );

			m_BloomTargets[i] = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(BLOOM_FORMAT, Size.X, Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_COMMON);

#if D3D12_Debug_INFO
			m_BloomTargets[i]->SetName( "Bloom" + GetDownSamplePostfix(i) + ((i%2 == 0) ? "Y" : "X") );
#endif

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = BLOOM_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;
			m_RTVHandles[i].CreateView(RenderTargetViewDesc, m_BloomTargets[i]->GetD3D12Resource());

			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = BLOOM_FORMAT;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = 0;
			m_SrvHandles[i].CreateView(Desc, m_BloomTargets[i]->GetD3D12Resource());
		}
	}

	void BloomBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void BloomBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void BloomBuffer::MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			IntPoint Size = IntPoint(m_Viewports[i].Width, m_Viewports[i].Height);
			Size = IntPoint::ComponentWiseMax(Size, IntPoint(1, 1));
			float InvSizeX = 1.0f / Size.X;
			float InvSizeY = 1.0f / Size.Y;

			//m_Data.SizeAndInvSize = Vector4(Size.X, Size.Y, 1.0f / Size.X, 1.0f / Size.Y);

			std::vector<float> SampleOffsets;
			SampleOffsets.resize(BLOOM_STATIC_SAMPLE_COUNT);

			float Weight = 1.0f / BLOOM_STATIC_SAMPLE_COUNT;
			Weight /= 5.0f;

			m_Data.SampleOffsetWeights.resize( BLOOM_PACKED_SAMPLE_COUNT );

			{
				for (int32 j = 0; j < BLOOM_PACKED_SAMPLE_COUNT - 1; j++)
				{
					m_Data.SampleOffsetWeights[j] = Vector4(InvSizeY * j, Weight, -InvSizeY * j, Weight);
				}
				m_Data.SampleOffsetWeights[BLOOM_PACKED_SAMPLE_COUNT - 1] = Vector4(0, Weight, 0, 0);

				m_Data.Header.SampleTexture = Renderer->m_SceneDownSampleBuffer->m_SrvHandles[i].GetIndex();

				UINT8* ConstantBufferStart;
				CD3DX12_RANGE readRange( 0, 0 );
				m_Buffer[i * 2]->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
				memcpy( ConstantBufferStart, &m_Data, sizeof(BloomDataHeader));
				memcpy( ConstantBufferStart + sizeof(BloomDataHeader), m_Data.SampleOffsetWeights.data(), sizeof(Vector4) * m_Data.SampleOffsetWeights.size());
				m_Buffer[i * 2]->GetD3D12Resource()->Unmap(0, nullptr);
			}

			{
				for (int32 j = 0; j < BLOOM_PACKED_SAMPLE_COUNT - 1; j++)
				{
					m_Data.SampleOffsetWeights[j] = Vector4(InvSizeX * j, Weight, -InvSizeX * j, Weight);
				}
				m_Data.SampleOffsetWeights[BLOOM_PACKED_SAMPLE_COUNT - 1] = Vector4(0, Weight, 0, 0);

				m_Data.Header.SampleTexture = m_SrvHandles[i * 2].GetIndex();
				m_Data.Header.AddtiveTexture = i == NUM_SCENE_DOWNSAMPLES - 1 ? 0 : m_SrvHandles[(i + 1) * 2 + 1].GetIndex();

				UINT8* ConstantBufferStart;
				CD3DX12_RANGE readRange( 0, 0 );
				m_Buffer[i * 2 + 1]->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
				memcpy( ConstantBufferStart, &m_Data, sizeof(BloomDataHeader));
				memcpy( ConstantBufferStart + sizeof(BloomDataHeader), m_Data.SampleOffsetWeights.data(), sizeof(Vector4) * m_Data.SampleOffsetWeights.size());
				m_Buffer[i * 2 + 1]->GetD3D12Resource()->Unmap(0, nullptr);
			}
		}
	}

	void BloomBuffer::ReleaseBuffers()
	{
		for (int32 i = 0; i < NUM_BLOOM_TARGETS; i++)
		{
			if (m_BloomTargets[i])
			{
				m_BloomTargets[i]->ReleaseBufferedResource();
				m_BloomTargets[i] = nullptr;
			}
		}

		for (int32 i = 0; i < NUM_BLOOM_TARGETS; i++)
		{
			if (m_Buffer[i])
			{
				m_Buffer[i]->ReleaseBufferedResource();
				m_Buffer[i] = nullptr;
			}
		}
	}

}