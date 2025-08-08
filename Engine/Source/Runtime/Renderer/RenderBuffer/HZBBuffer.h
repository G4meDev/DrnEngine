#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
	struct HZBViewHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
	};

	class HZBBuffer : public RenderBuffer
	{
	public:
		HZBBuffer();
		virtual ~HZBBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		void ReleaseResources();
		void ReallocateViewHandles();

		void TransitionSubresource(ID3D12GraphicsCommandList2* CommandList, int32 SubresourceIndex, D3D12_RESOURCE_STATES State);
		void TransitionAllSubresources(ID3D12GraphicsCommandList2* CommandList, D3D12_RESOURCE_STATES State);

		Resource* M_HZBTarget;
		std::vector<HZBViewHandle> m_UAVHandles;
		std::vector<HZBViewHandle> m_SrvHandles;
		std::vector<D3D12_RESOURCE_STATES> m_SubresourcesState;

		IntPoint m_FirstMipSize;
		int32 m_MipCount;
	};
}