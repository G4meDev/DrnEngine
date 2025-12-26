#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/ResourceView.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	struct BloomData
	{
		BloomData() = default;

		uint32 SampleTexture;
		uint32 AddtiveTexture;

		Vector2 Padding;

		Vector4 SampleOffsetWeights[BLOOM_PACKED_SAMPLE_COUNT];
	};

	class BloomBuffer : public RenderBuffer
	{
	public:
		BloomBuffer();
		virtual ~BloomBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		void MapBuffer( class D3D12CommandList* CommandList, SceneRenderer* Renderer );

		TRefCountPtr<RenderTexture2D> m_BloomTargets[NUM_BLOOM_TARGETS];

		TRefCountPtr<class RenderUniformBuffer> Buffer[NUM_BLOOM_TARGETS];
		BloomData m_Data;

		D3D12_VIEWPORT m_Viewports[NUM_SCENE_DOWNSAMPLES];

	private:

		float GaussianDistributionUnscaled(float X, float Sigma);

		inline std::string GetDownSamplePostfix(int32 Index) const { return "(1/" + std::to_string((int32)std::pow(2, Index + 2)) + ")"; }

	};
}