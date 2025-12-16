#include "DrnPCH.h"
#include "BloomBuffer.h"

#include "Runtime/Renderer/RenderBuffer/SceneDownSampleBuffer.h"

namespace Drn
{
	BloomBuffer::BloomBuffer()
	{}

	BloomBuffer::~BloomBuffer()
	{}

	void BloomBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void BloomBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < NUM_BLOOM_TARGETS; i++)
		{
			int32 DownSampleIndex = i / 2;
			IntPoint Size = m_Size / std::pow(2, DownSampleIndex + 1);
			Size = IntPoint::ComponentWiseMax(Size, IntPoint(1, 1));

			m_Viewports[DownSampleIndex] = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( Size.X ), static_cast<float>( Size.Y ) );

			RenderResourceCreateInfo BloomCreateInfo( nullptr, nullptr, ClearValueBinding::DepthZero, "Bloom" + GetDownSamplePostfix(i) + ((i%2 == 0) ? "Y" : "X") );
			m_BloomTargets[i] = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), Size.X, Size.Y, BLOOM_FORMAT, 1, 1, true,
				(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), BloomCreateInfo);
		}
	}

	void BloomBuffer::Clear( D3D12CommandList* CommandList )
	{
		
	}

	void BloomBuffer::Bind( D3D12CommandList* CommandList )
	{
		
	}

	void BloomBuffer::MapBuffer( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		std::vector<float> SampleWeights;
		SampleWeights.resize(BLOOM_PACKED_SAMPLE_COUNT);

		BloomSettings& Settings = Renderer->m_PostProcessSettings->m_BloomSettings;

		float SumWeights = 1;
		for (int32 i = 0; i < BLOOM_PACKED_SAMPLE_COUNT - 1; i++)
		{
			float a = BLOOM_PACKED_SAMPLE_COUNT;
			float X = (i + 1) / (float)(BLOOM_PACKED_SAMPLE_COUNT - 1);
			SampleWeights[i] = GaussianDistributionUnscaled(X, Settings.m_Sigma);
			SumWeights += SampleWeights[i] * 2;
		}

		SampleWeights[BLOOM_PACKED_SAMPLE_COUNT - 1] = GaussianDistributionUnscaled( 0.0f, Settings.m_Sigma );
		SumWeights += SampleWeights[BLOOM_PACKED_SAMPLE_COUNT - 1];

		for (int32 i = 0; i < BLOOM_PACKED_SAMPLE_COUNT; i++)
		{
			SampleWeights[i] = SampleWeights[i] * Settings.m_Brightness / SumWeights;
		}

		for (int32 i = 0; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			IntPoint Size = IntPoint(m_Viewports[i].Width, m_Viewports[i].Height);
			Size = IntPoint::ComponentWiseMax(Size, IntPoint(1, 1));

			float InvSizeX = Settings.m_Radius / Size.X;
			float InvSizeY = Settings.m_Radius/ Size.Y;

			std::vector<float> SampleOffsets;
			SampleOffsets.resize(BLOOM_STATIC_SAMPLE_COUNT);

			{
				for (int32 j = 0; j < BLOOM_PACKED_SAMPLE_COUNT - 1; j++)
				{
					m_Data.SampleOffsetWeights[j] = Vector4(InvSizeY * (j + 1), SampleWeights[j], -InvSizeY * (j + 1), SampleWeights[j]);
				}
				m_Data.SampleOffsetWeights[BLOOM_PACKED_SAMPLE_COUNT - 1] = Vector4(0, SampleWeights[BLOOM_PACKED_SAMPLE_COUNT - 1], 0, 0);

				m_Data.SampleTexture = Renderer->m_SceneDownSampleBuffer->m_DownSampleTargets[i]->GetShaderResourceView()->GetDescriptorHeapIndex();

				Buffer[i * 2] = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(BloomData), EUniformBufferUsage::SingleFrame, &m_Data);
			}

			{
				for (int32 j = 0; j < BLOOM_PACKED_SAMPLE_COUNT - 1; j++)
				{
					m_Data.SampleOffsetWeights[j] = Vector4(InvSizeX * (j + 1), SampleWeights[j], -InvSizeX * (j + 1), SampleWeights[j]);
				}
				m_Data.SampleOffsetWeights[BLOOM_PACKED_SAMPLE_COUNT - 1] = Vector4(0, SampleWeights[BLOOM_PACKED_SAMPLE_COUNT - 1], 0, 0);

				m_Data.SampleTexture = m_BloomTargets[i * 2]->GetShaderResourceView()->GetDescriptorHeapIndex();
				m_Data.AddtiveTexture = i == NUM_SCENE_DOWNSAMPLES - 1 ? 0 : m_BloomTargets[(i + 1) * 2 + 1]->GetShaderResourceView()->GetDescriptorHeapIndex();

				Buffer[i * 2 + 1] = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(BloomData), EUniformBufferUsage::SingleFrame, &m_Data);
			}
		}
	}

	float BloomBuffer::GaussianDistributionUnscaled( float X, float Sigma )
	{
		const float DX = 1 - std::abs(X);
		const float A = DX / Sigma;
		return std::exp(A * A);
	}

}