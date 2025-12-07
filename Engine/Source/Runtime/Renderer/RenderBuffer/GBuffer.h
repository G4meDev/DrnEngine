#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	class GBuffer : public RenderBuffer
	{
	public:
		GBuffer();
		virtual ~GBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void ClearDepth( D3D12CommandList* CommandList );
		virtual void Clear( D3D12CommandList* CommandList );
		//base pass
		virtual void Bind( D3D12CommandList* CommandList );
		virtual void BindDepth( D3D12CommandList* CommandList );

		virtual void BindLightPass( D3D12CommandList* CommandList );

		TRefCountPtr<RenderTexture2D> m_ColorDeferredTarget;
		TRefCountPtr<RenderTexture2D> m_BaseColorTarget;
		TRefCountPtr<RenderTexture2D> m_WorldNormalTarget;
		// Metallic Roughness AO Shading id
		TRefCountPtr<RenderTexture2D> m_MasksTarget;
		// Transmittance .etc
		TRefCountPtr<RenderTexture2D> m_MasksBTarget;
		TRefCountPtr<RenderTexture2D> m_VelocityTarget;

		TRefCountPtr<RenderTexture2D> m_DepthTarget;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

	private:
	};
}