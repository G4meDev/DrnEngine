#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	struct ScreenSpaceRefletcionData
	{
	public:
		ScreenSpaceRefletcionData() = default;

		uint32 DeferredColorTexture;
		uint32 BaseColorTexture;
		uint32 WorldNormalTexture;
		uint32 MasksTexture;
		uint32 DepthTexture;
		uint32 HzbTexture;
		float Intensity;
		float RoughnessFade;
	};

	class ScreenSpaceReflectionBuffer : public RenderBuffer
	{
	public:

		ScreenSpaceReflectionBuffer();
		virtual ~ScreenSpaceReflectionBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		void MapBuffer( class D3D12CommandList* CommandList, SceneRenderer* Renderer, const SSRSettings& Settings);
		void ReleaseBuffers();

		TRefCountPtr<RenderTexture2D> m_Target;

		Resource* m_Buffer;
		ScreenSpaceRefletcionData m_Data;
	};
}