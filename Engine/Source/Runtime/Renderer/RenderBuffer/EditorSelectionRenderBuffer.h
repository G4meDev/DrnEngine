#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
	class EditorSelectionRenderBuffer : public RenderBuffer
	{
	public:
		EditorSelectionRenderBuffer();
		virtual ~EditorSelectionRenderBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		Resource* m_DepthStencilTarget;
		D3D12_CLEAR_VALUE m_DepthStencilClearValue;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DepthStencilCpuHandle;


		D3D12_CPU_DESCRIPTOR_HANDLE m_DepthStencilSrvCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_DepthStencilSrvGpuHandle;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

	private:

	};
}