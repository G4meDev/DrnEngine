#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

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

		float Pad_3;
	};

	class RenderBufferAO : public RenderBuffer
	{
	public:
		RenderBufferAO();
		virtual ~RenderBufferAO();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;
		void BindSetup( ID3D12GraphicsCommandList2* CommandList );
		void BindHalf( ID3D12GraphicsCommandList2* CommandList );
		void BindMain( ID3D12GraphicsCommandList2* CommandList );
		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer, const SSAOSettings& Settings);

		void ReleaseBuffers();

		Resource* m_AOTarget;
		Resource* m_AOHalfTarget;
		Resource* m_AOSetupTarget;

		AoData m_AoData;
		Resource* m_AoBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_AORtvHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE m_AOHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_AOHalfHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_AOSetupHandle;

		D3D12_VIEWPORT m_Viewport;
		D3D12_VIEWPORT m_SetupViewport;
		D3D12_RECT     m_ScissorRect;

	private:
	};
}