#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

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

		uint32 DepthTexture;
		float CurrentFrameWeight;
		float CcurrentFrameVelocityWeight;
		float CcurrentFrameVelocityMultiplier;
	};

	class TAABuffer : public RenderBuffer
	{
	public:
		TAABuffer();
		virtual ~TAABuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		void MapBuffer( D3D12CommandList* CommandList, SceneRenderer* Renderer );

		TRefCountPtr<RenderTexture2D> m_TAATarget[2] = { nullptr, nullptr };
		TRefCountPtr<UnorderedAccessView> m_UavViews[2];

		TRefCountPtr<class RenderUniformBuffer> Buffer;
		TAAData m_Data;

		RenderTexture2D* GetFrameResource( uint32 FrameIndex ) const { return m_TAATarget[FrameIndex%2]; }
		RenderTexture2D* GetHistoryResource( uint32 FrameIndex ) const { return m_TAATarget[(FrameIndex+1)%2]; }

		UnorderedAccessView* GetFrameUAV( uint32 FrameIndex ) const { return m_UavViews[FrameIndex%2]; }
		UnorderedAccessView* GetHistoryUAV( uint32 FrameIndex ) const { return m_UavViews[(FrameIndex+1)%2]; }

		static Vector2 m_JitterOffsets[8];
	};
}