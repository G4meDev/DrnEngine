#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	struct AoData
	{
		AoData() = default;

		uint32 DepthTexture;
		uint32 WorldNormalTexture;
		uint32 HzbTexture;
		uint32 SetupTexture;

		uint32 DownSampleTexture;
		uint32 RandomTexture;
		float ToRandomU;
		float ToRandomV;

		float Intensity;
		float Power;
		float Bias;
		float Radius;

		float MipBlend;
		float InvFadeRadius;
		float FadeOffset;
		float Pad_1;

		Vector2 TemporalOffset;
		Vector2 Pad_2;
	};

	class RenderBufferAO : public RenderBuffer
	{
	public:
		RenderBufferAO();
		virtual ~RenderBufferAO();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( D3D12CommandList* CommandList );
		void Bind( D3D12CommandList* CommandList );
		void BindSetup( class D3D12CommandList* CommandList );
		void BindHalf( class D3D12CommandList* CommandList );
		void BindMain( class D3D12CommandList* CommandList );
		void MapBuffer( class D3D12CommandList* CommandList, SceneRenderer* Renderer, const SSAOSettings& Settings);

		TRefCountPtr<RenderTexture2D> m_AOTarget;
		TRefCountPtr<RenderTexture2D> m_AOHalfTarget;
		TRefCountPtr<RenderTexture2D> m_AOSetupTarget;

		AoData m_AoData;
		TRefCountPtr<class RenderUniformBuffer> AoBuffer;

		IntPoint HalfSize;

	private:
	};
}