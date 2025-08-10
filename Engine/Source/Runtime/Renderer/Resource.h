#pragma once

#include "ForwardTypes.h"
#include "BufferedResource.h"

namespace Drn
{
	class Resource : public BufferedResource
	{
	public:
		Resource();
		virtual ~Resource();

		static Resource* Create(D3D12_HEAP_TYPE HeapType, const D3D12_RESOURCE_DESC& ResourceDescription,
			D3D12_RESOURCE_STATES InitalState, bool NeedsStateTracking = true);

		static Resource* Create(D3D12_HEAP_TYPE HeapType, const D3D12_RESOURCE_DESC& ResourceDescription,
			D3D12_RESOURCE_STATES InitalState, const D3D12_CLEAR_VALUE& ClearValue, bool NeedsStateTracking = true);

		inline ID3D12Resource* GetD3D12Resource() { return m_Resource.Get(); }

		inline const D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return m_CpuHandle; }
		inline const D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_GpuHandle; }

#if D3D12_Debug_INFO
		void SetName(const std::string& Name);
#endif

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;

		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;

		bool m_StateTracking;
	};
}