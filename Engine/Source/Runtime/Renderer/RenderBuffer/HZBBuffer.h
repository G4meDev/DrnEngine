#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
	struct HZBMipUAVHandle
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

		void ReallocateUAVHandles();

		D3D12_CLEAR_VALUE m_ClearValue;

		Resource* M_HZBTarget;
		std::vector<HZBMipUAVHandle> m_UAVHandles;

		IntPoint m_FirstMipSize;
		int32 m_MipCount;
	};
}