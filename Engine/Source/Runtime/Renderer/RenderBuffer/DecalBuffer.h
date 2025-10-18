#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	class DecalBuffer : public RenderBuffer
	{
	public:
		DecalBuffer();
		virtual ~DecalBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		Resource* m_BaseColorTarget;
		DescriptorHandleRTV m_BaseColorRTVHandle;
		DescriptorHandleSRV m_BaseColorSRVHandle;

		Resource* m_NormalTarget;
		DescriptorHandleRTV m_NormalRTVHandle;
		DescriptorHandleSRV m_NormalSRVHandle;

		// Metallic Roughness AO
		Resource* m_MasksTarget;
		DescriptorHandleRTV m_MasksRTVHandle;
		DescriptorHandleSRV m_MasksSRVHandle;

		D3D12_CLEAR_VALUE m_BaseColorClearValue;
		D3D12_CLEAR_VALUE m_NormalClearValue;
		D3D12_CLEAR_VALUE m_MasksClearValue;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

	private:

		void ReleaseBuffers();
		void AllocateHandles();
		void ReleaseHandles();
	};
}