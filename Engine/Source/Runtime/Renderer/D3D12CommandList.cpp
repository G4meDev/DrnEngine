#include "DrnPCH.h"
#include "D3D12CommandList.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	int32 ResourceBarrierBatcher::AddTransition( class RenderResource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource )
	{
		drn_check( Before != After );

		if ( Barriers.size() )
		{
			const D3D12_RESOURCE_BARRIER& Last = Barriers.back();
			if ( pResource->GetResource() == Last.Transition.pResource &&
					Subresource == Last.Transition.Subresource && Before == Last.Transition.StateAfter &&
					After == Last.Transition.StateBefore &&
					Last.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION )
			{
				Barriers.pop_back();
				return -1;
			}
		}

		drn_check( IsValidD3D12ResourceState( Before ) && IsValidD3D12ResourceState( After ) );

		Barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition( pResource->GetResource(), Before, After, Subresource ));
		return 1;
	}

	void ResourceBarrierBatcher::Flush( ID3D12GraphicsCommandList* pCommandList, int32 BarrierBatchMax )
	{
		if (Barriers.size())
		{
			drn_check(pCommandList);
			if (Barriers.size() > BarrierBatchMax)
			{
				int Num = Barriers.size();
				D3D12_RESOURCE_BARRIER* Ptr = Barriers.data();
				while (Num > 0)
				{
					int DispatchNum = std::min(Num, BarrierBatchMax);
					pCommandList->ResourceBarrier(DispatchNum, Ptr);
					Ptr += BarrierBatchMax;
					Num -= BarrierBatchMax;
				}
			}
			else
			{
				pCommandList->ResourceBarrier(Barriers.size(), Barriers.data());
			}
		}

		Reset();
	}

	D3D12CommandList::D3D12CommandList( Device* Device, D3D12_COMMAND_LIST_TYPE Type, uint8 NumAllocators, const std::string& Name )
		: BufferedResource()
		, DeviceChild(Device)
		, m_Type(Type)
		, m_NumAllocators(NumAllocators)
		, m_CurrentAllocatorIndex(0)
	{
		m_CommandAllocators.resize(NumAllocators);

		for (uint8 i = 0; i < NumAllocators; i++)
		{
			Device->GetD3D12Device()->CreateCommandAllocator(Type, IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf()));

#if D3D12_Debug_INFO
			m_CommandAllocators[i]->SetName( StringHelper::s2ws( "CommandAllocator_" + Name + "_" + std::to_string(i)).c_str() );
#endif
		}

		Device->GetD3D12Device()->CreateCommandList(NULL, Type, m_CommandAllocators[0].Get(), NULL, IID_PPV_ARGS(m_CommandList.GetAddressOf()));

#if D3D12_Debug_INFO
		m_CommandList->SetName( StringHelper::s2ws("CommandList_" + Name).c_str() );
#endif
	}

	D3D12CommandList::~D3D12CommandList()
	{
		//m_ResourceBarrierBatcher.Flush(th)
	}

	void D3D12CommandList::Close()
	{
		m_CommandList->Close();
	}

	void D3D12CommandList::FlipAndReset()
	{
		m_CurrentAllocatorIndex = (m_CurrentAllocatorIndex + 1) % m_NumAllocators;
		auto commandAllocator = m_CommandAllocators[m_CurrentAllocatorIndex];
		commandAllocator->Reset();
		m_CommandList->Reset(commandAllocator.Get(), nullptr);
	}

	void D3D12CommandList::SetAllocatorAndReset( uint8 AllocatorIndex )
	{
		m_CurrentAllocatorIndex = AllocatorIndex;
		auto commandAllocator = m_CommandAllocators[m_CurrentAllocatorIndex];
		commandAllocator->Reset();
		m_CommandList->Reset(commandAllocator.Get(), nullptr);
	}

	void D3D12CommandList::ClearDepthTexture( class RenderTextureBase* InTexture, EDepthStencilViewType Type, bool bClearDepth, float Depth, bool bClearStencil, uint8 Stencil )
	{
		DepthStencilView* View = InTexture->GetDepthStencilView(Type);

		uint32 ClearFlags = 0;
		if (bClearDepth && View->HasDepth())
		{
			ClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
		}
		else if (bClearDepth)
		{
			drn_check(false);
		}

		if (bClearStencil && View->HasStencil())
		{
			ClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
		}
		else if (bClearStencil)
		{
			drn_check(false);
		}

		m_CommandList->ClearDepthStencilView( View->GetView(), (D3D12_CLEAR_FLAGS)ClearFlags, Depth, Stencil, 0, nullptr );
	}

	void D3D12CommandList::ClearDepthTexture( class RenderTextureBase* InTexture, EDepthStencilViewType Type, bool bClearDepth, bool bClearStencil )
	{
		ClearDepthTexture(InTexture, Type, bClearDepth, InTexture->GetDepthClearValue(), bClearStencil, InTexture->GetStencilClearValue());
	}

	void D3D12CommandList::ClearColorTexture( RenderTextureBase* InTexture, int32 MipIndex, int32 SliceIndex, Vector4 ClearValue )
	{
		m_CommandList->ClearRenderTargetView( InTexture->GetRenderTargetView(MipIndex, SliceIndex)->GetView(), (float*)(&(ClearValue)), 0, nullptr );
	}

	void D3D12CommandList::ClearColorTexture( RenderTextureBase* InTexture, int32 MipIndex, int32 SliceIndex )
	{
		ClearColorTexture(InTexture, MipIndex, SliceIndex, InTexture->GetClearColor());
	}

	void D3D12CommandList::AddTransitionBarrier( RenderResource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource )
	{
		drn_check(Before != After);
		m_ResourceBarrierBatcher.AddTransition(pResource, Before, After, Subresource);
	}

	void D3D12CommandList::TransitionResourceWithTracking( RenderResource* pResource, D3D12_RESOURCE_STATES After, uint32 Subresource )
	{
		drn_check(pResource);
		drn_check(pResource->RequiresResourceStateTracking());
		drn_check(!((After & (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) && (pResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)));

		//ResourceState_New& ResourceState = GetResourceState(pResource);
		//if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !ResourceState.AreAllSubresourcesSame())
		//{
		//	const uint8 SubresourceCount = pResource->GetSubresourceCount();
		//	for (uint32 SubresourceIndex = 0; SubresourceIndex < SubresourceCount; SubresourceIndex++)
		//	{
		//		const D3D12_RESOURCE_STATES before = ResourceState.GetSubresourceState(SubresourceIndex);
		//		if (before == D3D12_RESOURCE_STATE_TBD)
		//		{
		//			AddPendingResourceBarrier(pResource, after, SubresourceIndex);
		//			ResourceState.SetSubresourceState(SubresourceIndex, after);
		//		}
		//		else if (before != after)
		//		{
		//			AddTransitionBarrier(pResource, before, after, SubresourceIndex);
		//			ResourceState.SetSubresourceState(SubresourceIndex, after);
		//		}
		//	}
		//
		//	// The entire resource should now be in the after state on this command list (even if all barriers are pending)
		//	drn_check(ResourceState.CheckResourceState(after));
		//	ResourceState.SetResourceState(after);
		//}
		//else
		//{
		//	const D3D12_RESOURCE_STATES before = ResourceState.GetSubresourceState(subresource);
		//	if (before == D3D12_RESOURCE_STATE_TBD)
		//	{
		//		AddPendingResourceBarrier(pResource, after, subresource);
		//		ResourceState.SetSubresourceState(subresource, after);
		//	}
		//	else if (IsTransitionNeeded(before, after))
		//	{
		//		AddTransitionBarrier(pResource, before, after, subresource);
		//		ResourceState.SetSubresourceState(subresource, after);
		//	}
		//}

		//TODO: add pending transition for start of command list

		ResourceState_New& State = pResource->GetResourceState();
		if (Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !State.AreAllSubresourcesSame())
		{
			const uint8 SubresourceCount = pResource->GetSubresourceCount();
			for (uint32 SubresourceIndex = 0; SubresourceIndex < SubresourceCount; SubresourceIndex++)
			{
				const D3D12_RESOURCE_STATES Before = State.GetSubresourceState(SubresourceIndex);
				if (Before != After)
				{
					AddTransitionBarrier(pResource, Before, After, SubresourceIndex);
					State.SetSubresourceState(SubresourceIndex, After);
				}
			}

			// The entire resource should now be in the after state on this command list (even if all barriers are pending)
			drn_check(State.CheckResourceState(After));
			State.SetResourceState(After);
		}
		else
		{
			const D3D12_RESOURCE_STATES Before = State.GetSubresourceState(Subresource);
			if (IsTransitionNeeded(Before, After))
			{
				AddTransitionBarrier(pResource, Before, After, Subresource);
				State.SetSubresourceState(Subresource, After);
			}
		}
	}

	//void D3D12CommandList::TransitionResourceWithTracking( class RenderResource* pResource, D3D12_RESOURCE_STATES After, const CViewSubresourceSubset& SubresourceSubset )
	//{
	//	//TODO: add pending transition for start of command list
	//
	//}

	ResourceState_New& D3D12CommandList::GetResourceState( class RenderResource* pResource )
	{
		return TrackedResourceState.GetResourceState(pResource);
	}

	void D3D12CommandList::FlushBarriers()
	{
		m_ResourceBarrierBatcher.Flush(m_CommandList.Get());
	}

	void CommandListResourceState::ConditionalInitalize( RenderResource* pResource, ResourceState_New& ResourceState )
	{
		if (!ResourceState.CheckResourceStateInitalized())
		{
			ResourceState.Initialize(pResource->GetSubresourceCount());
			drn_check(ResourceState.CheckResourceState(D3D12_RESOURCE_STATE_TBD));
		}

		drn_check(ResourceState.CheckResourceStateInitalized());
	}

	ResourceState_New& CommandListResourceState::GetResourceState( RenderResource* pResource )
	{
		drn_check(pResource->RequiresResourceStateTracking());

		auto it = ResourceStates.insert({pResource, ResourceState_New()}).first;
		ResourceState_New& OutResourceState = it->second;

		ConditionalInitalize(pResource, OutResourceState);
		return OutResourceState;
	}

	void CommandListResourceState::Empty()
	{
		ResourceStates.clear();
	}

	bool IsTransitionNeeded( D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES& After )
	{
		drn_check( Before != D3D12_RESOURCE_STATE_CORRUPT && After != D3D12_RESOURCE_STATE_CORRUPT );
		drn_check( Before != D3D12_RESOURCE_STATE_TBD && After != D3D12_RESOURCE_STATE_TBD );

		if ( ( Before == D3D12_RESOURCE_STATE_DEPTH_WRITE ) && ( After == D3D12_RESOURCE_STATE_DEPTH_READ ) )
		{
			return false;
		}

		if ( After == D3D12_RESOURCE_STATE_COMMON )
		{
			return ( Before != D3D12_RESOURCE_STATE_COMMON );
		}

		D3D12_RESOURCE_STATES Combined = Before | After;
		if ( ( Combined & ( D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT ) ) == Combined )
		{
			After = Combined;
		}

		return Before != After;
	}

}  // namespace Drn