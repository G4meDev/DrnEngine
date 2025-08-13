#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"

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

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer, const SSRSettings& Settings);
		void ReleaseBuffers();

		Resource* m_Target;
		D3D12_CLEAR_VALUE m_ClearValue;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_Handle;

		Resource* m_Buffer;
		ScreenSpaceRefletcionData m_Data;
	};
}