#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
	class EditorPrimitiveRenderBuffer : public RenderBuffer
	{
	public:
		EditorPrimitiveRenderBuffer();
		virtual ~EditorPrimitiveRenderBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		Resource* m_ColorTarget;
		Resource* m_DepthTarget;

		D3D12_CLEAR_VALUE m_ColorClearValue;
		D3D12_CLEAR_VALUE m_DepthClearValue;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_ColorRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DepthSrvHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE m_ColorCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DepthCpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE m_ColorSrvCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ColorSrvGpuHandle;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

	private:
	};
}