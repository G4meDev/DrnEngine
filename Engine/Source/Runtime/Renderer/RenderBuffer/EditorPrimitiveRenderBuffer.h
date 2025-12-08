#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	class EditorPrimitiveRenderBuffer : public RenderBuffer
	{
	public:
		EditorPrimitiveRenderBuffer();
		virtual ~EditorPrimitiveRenderBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		TRefCountPtr<RenderTexture2D> m_ColorTarget;
		TRefCountPtr<RenderTexture2D> m_DepthTarget;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

	private:
	};
}