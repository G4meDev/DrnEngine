#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	struct GBufferTextures
	{
		uint32 DepthIndex;
		uint32 DeferredColorIndex;
		uint32 BaseColorIndex;
		uint32 NormalIndex;
		uint32 MasksAIndex;
		uint32 MasksBIndex;
		uint32 VelocityIndex;
	};

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

		TRefCountPtr<RenderTexture2D> m_SeparateTranslucencyTarget;
		TRefCountPtr<RenderTexture2D> m_SceneColorTarget;
		TRefCountPtr<RenderTexture2D> m_DistortedSceneColorTarget;

		inline class RenderUniformBuffer* GetTexturesBuffer(D3D12CommandList* CommandList)
		{
			if (bTexturesBufferDirty)
			{
				UpdateTexturesBuffer(CommandList);
				bTexturesBufferDirty = false;
			}
			return TexturesBuffer;
		}

		void TransitionTexturesToRead(D3D12CommandList* CommandList);

	private:
		void UpdateTexturesBuffer(D3D12CommandList* CommandList);

		TRefCountPtr<class RenderUniformBuffer> TexturesBuffer;
		GBufferTextures Textures;

		bool bTexturesBufferDirty;
	};
}