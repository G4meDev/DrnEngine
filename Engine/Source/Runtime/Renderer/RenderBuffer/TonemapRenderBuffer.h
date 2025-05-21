#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
	class TonemapRenderBuffer : public RenderBuffer
	{
	public:
		TonemapRenderBuffer();
		virtual ~TonemapRenderBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		Resource* m_TonemapTarget;

		D3D12_CLEAR_VALUE m_TonemapClearValue;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_TonemapRtvHeap;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

	private:

	};
}