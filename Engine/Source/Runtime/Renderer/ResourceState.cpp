#include "DrnPCH.h"
#include "ResourceState.h"

namespace Drn
{
	void ResourceState::Initialize( uint32 SubresourceCount )
	{
		drn_check(0 == m_SubresourceState.size());
		drn_check(SubresourceCount > 0);

		m_SubresourceState.resize(SubresourceCount);

		SetResourceState(D3D12_RESOURCE_STATE_TBD);
	}

	bool ResourceState::AreAllSubresourcesSame() const
	{
		return m_AllSubresourcesSame;
	}

	bool ResourceState::CheckResourceState( D3D12_RESOURCE_STATES State ) const
	{
		if (m_AllSubresourcesSame)
		{
			return State == m_ResourceState;
		}
		else
		{
			const uint32 numSubresourceStates = m_SubresourceState.size();
			for (uint32 i = 0; i < numSubresourceStates; i++)
			{
				if (m_SubresourceState[i] != State)
				{
					return false;
				}
			}

			return true;
		}
	}

	bool ResourceState::CheckResourceStateInitalized() const
	{
		return m_SubresourceState.size() > 0;
	}

	D3D12_RESOURCE_STATES ResourceState::GetSubresourceState( uint32 SubresourceIndex ) const
	{
		if (m_AllSubresourcesSame)
		{
			return static_cast<D3D12_RESOURCE_STATES>(m_ResourceState);
		}
		else
		{
			drn_check(SubresourceIndex < static_cast<uint32>(m_SubresourceState.size()));
			return m_SubresourceState[SubresourceIndex];
		}
	}

	bool ResourceState::CheckAllSubresourceSame()
	{
		if (m_AllSubresourcesSame)
		{
			return true;
		}
		else
		{
			D3D12_RESOURCE_STATES State = m_SubresourceState[0];

			const uint32 numSubresourceStates = m_SubresourceState.size();
			for (uint32 i = 1; i < numSubresourceStates; i++)
			{
				if (m_SubresourceState[i] != State)
				{
					return false;
				}
			}

			SetResourceState(State);

			return true;
		}
	}

	void ResourceState::SetResourceState( D3D12_RESOURCE_STATES State )
	{
		m_AllSubresourcesSame = 1;
		drn_check((State & (1 << 31)) == 0);

		m_ResourceState = *reinterpret_cast<uint32*>(&State);
	}

	void ResourceState::SetSubresourceState( uint32 SubresourceIndex, D3D12_RESOURCE_STATES State )
	{
		if (SubresourceIndex == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES || m_SubresourceState.size() == 1)
		{
			SetResourceState(State);
		}
		else
		{
			drn_check(SubresourceIndex < static_cast<uint32>(m_SubresourceState.size()));

			if (m_AllSubresourcesSame)
			{
				const uint32 numSubresourceStates = m_SubresourceState.size();
				for (uint32 i = 0; i < numSubresourceStates; i++)
				{
					m_SubresourceState[i] = static_cast<D3D12_RESOURCE_STATES>(m_ResourceState);
				}

				m_AllSubresourcesSame = 0;
			}

			m_SubresourceState[SubresourceIndex] = State;
		}
	}

}