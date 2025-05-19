#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

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

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

	protected:

		Resource* m_GuidTarget;
		Resource* m_DepthTarget;

		D3D12_CLEAR_VALUE m_GuidClearValue;
		D3D12_CLEAR_VALUE m_DepthClearValue;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_GuidDescriptorHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DepthHeap;
		//D3D12_CPU_DESCRIPTOR_HANDLE m_GuidCpuHandle;

	private:

	};
}

#endif