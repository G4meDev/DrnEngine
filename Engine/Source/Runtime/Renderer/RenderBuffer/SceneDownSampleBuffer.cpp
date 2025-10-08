#include "DrnPCH.h"
#include "SceneDownSampleBuffer.h"

#include "Runtime/Renderer/RenderBuffer/TAABuffer.h"

namespace Drn
{
	SceneDownSampleBuffer::SceneDownSampleBuffer()
	{
		
	}

	SceneDownSampleBuffer::~SceneDownSampleBuffer()
	{
		ReleaseBuffers();

		for ( int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			m_RTVHandles[i].FreeDescriptorSlot();
			m_SrvHandles[i].FreeDescriptorSlot();
		}
	}

	void SceneDownSampleBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			m_RTVHandles[i].AllocateDescriptorSlot();
			m_SrvHandles[i].AllocateDescriptorSlot();
		}

// ---------------------------------------------------------------------------------------------------------------

		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			m_Buffer[i] = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
			m_Buffer[i]->SetName("CB_SceneDownSample" + GetDownSamplePostfix(i));
#endif

			D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
			ResourceViewDesc.BufferLocation = m_Buffer[i]->GetD3D12Resource()->GetGPUVirtualAddress();
			ResourceViewDesc.SizeInBytes = 256;
			Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_Buffer[i]->GetCpuHandle());
		}
	}

	void SceneDownSampleBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			if (m_DownSampleTargets[i])
			{
				m_DownSampleTargets[i]->ReleaseBufferedResource();
				m_DownSampleTargets[i] = nullptr;
			}
		}

		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			IntPoint Size = m_Size / std::pow(2, i + 1);
			Size = IntPoint::ComponentWiseMax(Size, IntPoint(1, 1));

			m_Viewports[i] = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( Size.X ), static_cast<float>( Size.Y ) );

			m_DownSampleTargets[i] = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(SCENE_DOWN_SAMPLE_FORMAT, Size.X, Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_COMMON);

#if D3D12_Debug_INFO
			m_DownSampleTargets[i]->SetName( "SceneDownSample" + GetDownSamplePostfix(i) );
#endif

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = SCENE_DOWN_SAMPLE_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;
			m_RTVHandles[i].CreateView(RenderTargetViewDesc, m_DownSampleTargets[i]->GetD3D12Resource());

			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = SCENE_DOWN_SAMPLE_FORMAT;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = 0;
			m_SrvHandles[i].CreateView(Desc, m_DownSampleTargets[i]->GetD3D12Resource());
		}
	}

	void SceneDownSampleBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void SceneDownSampleBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void SceneDownSampleBuffer::MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			IntPoint ParentSize = m_Size / std::pow(2, i);
			ParentSize = IntPoint::ComponentWiseMax(ParentSize, IntPoint(1, 1));

			m_Data.ParentSizeAndInvSize = Vector4(ParentSize.X, ParentSize.Y, 1.0f / ParentSize.X, 1.0f / ParentSize.Y);

			m_Data.ParentTexture = i == 0
				? Renderer->m_TAABuffer->GetFrameSRV(Renderer->m_FrameIndex).GetIndex()
				: m_SrvHandles[i - 1].GetIndex();

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_Buffer[i]->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &m_Data, sizeof(SceneDownSampleData));
			m_Buffer[i]->GetD3D12Resource()->Unmap(0, nullptr);
		}
	}

	void SceneDownSampleBuffer::ReleaseBuffers()
	{
		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			if (m_DownSampleTargets[i])
			{
				m_DownSampleTargets[i]->ReleaseBufferedResource();
				m_DownSampleTargets[i] = nullptr;
			}
		}

		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			if (m_Buffer[i])
			{
				m_Buffer[i]->ReleaseBufferedResource();
				m_Buffer[i] = nullptr;
			}
		}
	}

}