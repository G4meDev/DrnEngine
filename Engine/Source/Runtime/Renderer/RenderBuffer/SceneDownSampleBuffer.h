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
		Vector Padding;
	};

	class SceneDownSampleBuffer : public RenderBuffer
	{
	public:
		SceneDownSampleBuffer();
		virtual ~SceneDownSampleBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& InSize ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		void MapBuffer( class D3D12CommandList* CommandList, SceneRenderer* Renderer );

		TRefCountPtr<RenderTexture2D> m_DownSampleTargets[NUM_SCENE_DOWNSAMPLES] = { nullptr };

		TRefCountPtr<class RenderUniformBuffer> Buffer[NUM_SCENE_DOWNSAMPLES];
		SceneDownSampleData m_Data;

		IntPoint m_Viewports[NUM_SCENE_DOWNSAMPLES];

	private:

#if D3D12_Debug_INFO
		inline std::string GetDownSamplePostfix(int32 Index) const { return "(1/" + std::to_string((int32)std::pow(2, Index + 2)) + ")"; }
#endif

	};
}