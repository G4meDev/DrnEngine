#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	struct SceneDownSampleData
	{
	public:
		SceneDownSampleData() = default;

		Vector4 ParentSizeAndInvSize;

		uint32 ParentTexture;
	};

	class SceneDownSampleBuffer : public RenderBuffer
	{
	public:
		SceneDownSampleBuffer();
		virtual ~SceneDownSampleBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer );
		void ReleaseBuffers();

		TRefCountPtr<RenderTexture2D> m_DownSampleTargets[NUM_SCENE_DOWNSAMPLES] = { nullptr };

		Resource* m_Buffer[NUM_SCENE_DOWNSAMPLES] = {nullptr};
		SceneDownSampleData m_Data;

		D3D12_VIEWPORT m_Viewports[NUM_SCENE_DOWNSAMPLES];

	private:

#if D3D12_Debug_INFO
		inline std::string GetDownSamplePostfix(int32 Index) const { return "(1/" + std::to_string((int32)std::pow(2, Index + 2)) + ")"; }
#endif

	};
}