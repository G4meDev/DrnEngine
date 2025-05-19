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
			D3D12_RESOURCE_STATES InitalState );

		static Resource* Create(D3D12_HEAP_TYPE HeapType, const D3D12_RESOURCE_DESC& ResourceDescription,
			D3D12_RESOURCE_STATES InitalState, const D3D12_CLEAR_VALUE& ClearValue );

		inline ID3D12Resource* GetD3D12Resource() { return m_Resource.Get(); }

#if D3D12_Debug_INFO
		void SetName(const std::string& Name);
#endif

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
	};
}