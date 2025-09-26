#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	struct TAAData
	{
	public:
		TAAData() = default;

		uint32 DeferredColorTexture;
		uint32 VelocityTexture;
		uint32 HistoryTexture;
		uint32 TargetTexture;
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

		Resource* m_TAATarget[2] = { nullptr, nullptr };

		DescriptorHandleUAV m_UAVHandles[2];
		DescriptorHandleSRV m_SrvHandles[2];

		Resource* m_Buffer;
		TAAData m_Data;

		Resource* GetFrameResource( uint32 FrameIndex ) const { return m_TAATarget[FrameIndex%2]; }
		Resource* GetHistoryResource( uint32 FrameIndex ) const { return m_TAATarget[(FrameIndex+1)%2]; }

		DescriptorHandleUAV GetFrameUAV( uint32 FrameIndex ) const { return m_UAVHandles[FrameIndex%2]; }
		DescriptorHandleUAV GetHistoryUAV( uint32 FrameIndex ) const { return m_UAVHandles[(FrameIndex+1)%2]; }

		DescriptorHandleSRV GetFrameSRV( uint32 FrameIndex ) const { return m_SrvHandles[FrameIndex%2]; }
		DescriptorHandleSRV GetHistorySRV( uint32 FrameIndex ) const { return m_SrvHandles[(FrameIndex+1)%2]; }

		// TODO: make vector2d
		static float m_JitterOffsets[8][2];
	};
}