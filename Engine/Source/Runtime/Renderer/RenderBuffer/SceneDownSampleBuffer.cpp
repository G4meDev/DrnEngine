#include "DrnPCH.h"
#include "SceneDownSampleBuffer.h"

#include "Runtime/Renderer/RenderBuffer/TAABuffer.h"

namespace Drn
{
	SceneDownSampleBuffer::SceneDownSampleBuffer()
	{}

	SceneDownSampleBuffer::~SceneDownSampleBuffer()
	{}

	void SceneDownSampleBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void SceneDownSampleBuffer::Resize( const IntPoint& InSize )
	{
		RenderBuffer::Resize(InSize);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			IntPoint Size = m_Size / std::pow(2, i + 1);
			Size = IntPoint::ComponentWiseMax(Size, IntPoint(1, 1));

			m_Viewports[i] = Size;

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

	void SceneDownSampleBuffer::MapBuffer( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			IntPoint ParentSize = m_Size / std::pow(2, i);
			ParentSize = IntPoint::ComponentWiseMax(ParentSize, IntPoint(1, 1));

			m_Data.ParentSizeAndInvSize = Vector4(ParentSize.X, ParentSize.Y, 1.0f / ParentSize.X, 1.0f / ParentSize.Y);

			m_Data.ParentTexture = i == 0
				? Renderer->m_TAABuffer->GetFrameResource(Renderer->m_FrameIndex)->GetShaderResourceView()->GetDescriptorHeapIndex()
				: m_DownSampleTargets[i - 1]->GetShaderResourceView()->GetDescriptorHeapIndex();

			Buffer[i] = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(SceneDownSampleData), EUniformBufferUsage::SingleFrame, &m_Data);
		}
	}

}