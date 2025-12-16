#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ResourceState
	{
	public:
		void Initialize(uint32 SubresourceCount);

		bool AreAllSubresourcesSame() const;
		bool CheckResourceState(D3D12_RESOURCE_STATES State) const;
		bool CheckResourceStateInitalized() const;
		D3D12_RESOURCE_STATES GetSubresourceState(uint32 SubresourceIndex) const;
		bool CheckAllSubresourceSame();
		void SetResourceState(D3D12_RESOURCE_STATES State);
		void SetSubresourceState(uint32 SubresourceIndex, D3D12_RESOURCE_STATES State);

	private:
		uint32 m_ResourceState : 31;
		uint32 m_AllSubresourcesSame : 1;

		std::vector<D3D12_RESOURCE_STATES> m_SubresourceState;
	};
}