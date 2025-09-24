#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"

namespace Drn
{
	struct TAAData
	{
	public:
		TAAData() = default;

		uint32 DeferredColorTexture;
		uint32 HistoryTexture;
	};

	class TAABuffer : public RenderBuffer
	{
	public:
		TAABuffer();
		virtual ~TAABuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer );
		void ReleaseBuffers();

		Resource* m_TAATarget;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;

		Resource* m_Buffer;
		TAAData m_Data;

		// TODO: make vector2d
		static float m_JitterOffsets[4][2];
	};
}