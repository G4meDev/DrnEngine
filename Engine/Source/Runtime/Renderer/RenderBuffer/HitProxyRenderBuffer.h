#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

#if WITH_EDITOR

namespace Drn
{
	class HitProxyRenderBuffer : public RenderBuffer
	{
	public:
		HitProxyRenderBuffer();
		virtual ~HitProxyRenderBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		TRefCountPtr<RenderTexture2D> m_GuidTarget;
		TRefCountPtr<RenderTexture2D> m_DepthTarget;

	private:

	};
}

#endif