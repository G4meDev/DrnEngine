#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/ResourceView.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	struct BloomDataHeader
	{
		BloomDataHeader() = default;

		uint32 SampleTexture;
		uint32 AddtiveTexture;

		float Pad_1;
		float Pad_2;
	};

	struct BloomData
	{
		BloomData() = default;

		BloomDataHeader Header;
		std::vector<Vector4> SampleOffsetWeights;
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

		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer );
		void ReleaseBuffers();

		TRefCountPtr<RenderTexture2D> m_BloomTargets[NUM_BLOOM_TARGETS] = { nullptr };

		Resource* m_Buffer[NUM_BLOOM_TARGETS] = { nullptr };
		BloomData m_Data;

		D3D12_VIEWPORT m_Viewports[NUM_SCENE_DOWNSAMPLES];

	private:

		float GaussianDistributionUnscaled(float X, float Sigma);

#if D3D12_Debug_INFO
		inline std::string GetDownSamplePostfix(int32 Index) const { return "(1/" + std::to_string((int32)std::pow(2, Index + 2)) + ")"; }
#endif

	};
}