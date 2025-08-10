#include "DrnPCH.h"
#include "ResourceStateTracker.h"

LOG_DEFINE_CATEGORY( LogResourceStateTracker, "ResourceStateTracker" )

namespace Drn
{
	ResourceStateTracker* ResourceStateTracker::m_SingletonInstance = new ResourceStateTracker();

	void ResourceStateTracker::RegisterResource( ID3D12Resource* InResource, D3D12_RESOURCE_STATES State )
	{
		if (InResource)
			m_ResourceStateMap[InResource].SetSubresourceState(State);
	}

	void ResourceStateTracker::UnRegisterResource( ID3D12Resource* InResource )
	{
		if (InResource)
		{
			const auto& It = m_ResourceStateMap.find(InResource);
			if (It != m_ResourceStateMap.end())
				m_ResourceStateMap.erase(It);
		}
	}

	void ResourceStateTracker::TransiationResource( ID3D12Resource* InResource, D3D12_RESOURCE_STATES StateAfter, uint32 Subresource )
	{
		if (InResource)
		{
			const auto It = m_ResourceStateMap.find(InResource);
			if (It == m_ResourceStateMap.end())
				__debugbreak();

			ResourceState& State = It->second;
			if (Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !State.m_SubresourcesState.empty())
			{
				// TODO: add helper for subresource count
				const uint32 NumSubresources = InResource->GetDesc().MipLevels;

				for (uint32 i = 0; i < NumSubresources; i++)
				{
					D3D12_RESOURCE_STATES FinalState = State.GetSubresourceState(i);
					if (StateAfter != FinalState)
					{
						m_ResourceBarriers.emplace_back( CD3DX12_RESOURCE_BARRIER::Transition(InResource, FinalState, StateAfter, i) );
					}
				}

				State.SetSubresourceState( StateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES );
			}

			else
			{
				D3D12_RESOURCE_STATES FinalState = State.GetSubresourceState(Subresource);
				if (StateAfter != FinalState)
				{
					m_ResourceBarriers.emplace_back( CD3DX12_RESOURCE_BARRIER::Transition(InResource, FinalState, StateAfter, Subresource) );
					State.SetSubresourceState(StateAfter, Subresource);
				}
			}
		}
	}

	void ResourceStateTracker::TransiationResource( Resource* InResource, D3D12_RESOURCE_STATES StateAfter, uint32 Subresource )
	{
		if (InResource)
			TransiationResource(InResource->GetD3D12Resource(), StateAfter, Subresource);
	}

	void ResourceStateTracker::FlushResourceBarriers( ID3D12GraphicsCommandList2* CommandList )
	{
		const uint32 BarrierCount = m_ResourceBarriers.size();
		if (CommandList && BarrierCount > 0)
		{
			CommandList->ResourceBarrier(BarrierCount, m_ResourceBarriers.data());
			m_ResourceBarriers.clear();
		}
	}

}