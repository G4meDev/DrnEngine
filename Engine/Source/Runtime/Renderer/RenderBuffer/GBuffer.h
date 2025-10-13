#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
	class GBuffer : public RenderBuffer
	{
	public:
		GBuffer();
		virtual ~GBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void ClearDepth( ID3D12GraphicsCommandList2* CommandList );
		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		//base pass
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void BindDepth( ID3D12GraphicsCommandList2* CommandList );

		virtual void BindLightPass( ID3D12GraphicsCommandList2* CommandList );

		Resource* m_ColorDeferredTarget;
		Resource* m_BaseColorTarget;
		Resource* m_WorldNormalTarget;
		// Metallic Roughness AO Shading id
		Resource* m_MasksTarget;
		Resource* m_VelocityTarget;
		Resource* m_DepthTarget;

		D3D12_CLEAR_VALUE m_ColorDeferredClearValue;
		D3D12_CLEAR_VALUE m_BaseColorClearValue;
		D3D12_CLEAR_VALUE m_WorldNormalClearValue;
		D3D12_CLEAR_VALUE m_MasksClearValue;
		D3D12_CLEAR_VALUE m_VelocityClearValue;
		D3D12_CLEAR_VALUE m_DepthClearValue;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE m_ColorDeferredCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_BaseColorCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_WorldNormalCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_MasksCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_VelocityCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DepthCpuHandle;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

	private:
	};
}