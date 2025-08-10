#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogResourceStateTracker)

namespace Drn
{
	class ResourceState
	{
	public:

		ResourceState(D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON)
			: m_State(State)
		{
		}

		void SetSubresourceState( D3D12_RESOURCE_STATES State, uint32 Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES )
		{
			if (Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
			{
				m_State = State;
				m_SubresourcesState.clear();
			}
			else
			{
				m_SubresourcesState[Subresource] = State;
			}
		}

		D3D12_RESOURCE_STATES GetSubresourceState(uint32 Subresource)
		{
			const auto It = m_SubresourcesState.find(Subresource);
			return It != m_SubresourcesState.end() ? It->second : m_State;
		}

		D3D12_RESOURCE_STATES m_State;
		std::map<uint32, D3D12_RESOURCE_STATES> m_SubresourcesState;
	};

	class ResourceStateTracker
	{
	public:

		void RegisterResource( ID3D12Resource* InResource, D3D12_RESOURCE_STATES State );
		void UnRegisterResource( ID3D12Resource* InResource);

		void TransiationResource( ID3D12Resource* InResource, D3D12_RESOURCE_STATES StateAfter, uint32 Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES );
		void TransiationResource( Resource* InResource, D3D12_RESOURCE_STATES StateAfter, uint32 Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES );

		void FlushResourceBarriers( ID3D12GraphicsCommandList2* CommandList );

		inline static ResourceStateTracker* Get() { return m_SingletonInstance; };

	private:

		std::vector<D3D12_RESOURCE_BARRIER> m_ResourceBarriers;
		std::unordered_map<ID3D12Resource*, ResourceState> m_ResourceStateMap;

		static ResourceStateTracker* m_SingletonInstance;
	};
}