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

	}

	void SceneDownSampleBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

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
			IntPoint Size = m_Size / std::pow(2, i + 1);
			Size = IntPoint::ComponentWiseMax(Size, IntPoint(1, 1));

			m_Viewports[i] = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( Size.X ), static_cast<float>( Size.Y ) );

			RenderResourceCreateInfo DownSampleCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "SceneDownSample" + GetDownSamplePostfix(i) );
			m_DownSampleTargets[i] = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), Size.X, Size.Y, SCENE_DOWN_SAMPLE_FORMAT, 1, 1, true,
				(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), DownSampleCreateInfo);
		}
	}

	void SceneDownSampleBuffer::Clear( D3D12CommandList* CommandList )
	{
		
	}

	void SceneDownSampleBuffer::Bind( D3D12CommandList* CommandList )
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
				? Renderer->m_TAABuffer->GetFrameResource(Renderer->m_FrameIndex)->GetShaderResourceView()->GetDescriptorHeapIndex()
				: m_DownSampleTargets[i - 1]->GetShaderResourceView()->GetDescriptorHeapIndex();

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
			if (m_Buffer[i])
			{
				m_Buffer[i]->ReleaseBufferedResource();
				m_Buffer[i] = nullptr;
			}
		}
	}

}