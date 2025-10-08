#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	struct BloomData
	{
	public:
		BloomData() = default;

		Vector4 SizeAndInvSize;

		uint32 SampleTexture;
		uint32 AddtiveTexture;
	};

	class BloomBuffer : public RenderBuffer
	{
	public:
		BloomBuffer();
		virtual ~BloomBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer );
		void ReleaseBuffers();

		Resource* m_BloomTargets[NUM_BLOOM_TARGETS] = { nullptr };

		DescriptorHandleRTV m_RTVHandles[NUM_BLOOM_TARGETS];
		DescriptorHandleSRV m_SrvHandles[NUM_BLOOM_TARGETS];

		Resource* m_Buffer[NUM_BLOOM_TARGETS] = { nullptr };
		BloomData m_Data;

		D3D12_VIEWPORT m_Viewports[NUM_SCENE_DOWNSAMPLES];

	private:

#if D3D12_Debug_INFO
		inline std::string GetDownSamplePostfix(int32 Index) const { return "(1/" + std::to_string((int32)std::pow(2, Index + 2)) + ")"; }
#endif

	};
}