#include "DrnPCH.h"
#include "D3D12CommandList.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	static void GetReadBackHeapDescImpl(D3D12_PLACED_SUBRESOURCE_FOOTPRINT& OutFootprint, ID3D12Device* InDevice, D3D12_RESOURCE_DESC const& InResourceDesc, uint32 InSubresource)
	{
		uint64 Offset = 0;
		if (InSubresource > 0)
		{
			InDevice->GetCopyableFootprints(&InResourceDesc, 0, InSubresource, 0, nullptr, nullptr, nullptr, &Offset);
			Offset = Align(Offset, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		}
		InDevice->GetCopyableFootprints(&InResourceDesc, InSubresource, 1, Offset, &OutFootprint, nullptr, nullptr, nullptr);

		drn_check(OutFootprint.Footprint.Width > 0 && OutFootprint.Footprint.Height > 0);
	}

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
		: SimpleRenderResource()
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

		StateCache.Init(Device, this, nullptr);
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

	void D3D12CommandList::ClearDepthTexture( class DepthStencilView* InView, bool bClearDepth, float Depth, bool bClearStencil, uint8 Stencil )
	{
		uint32 Flags = 0;
		Flags |= bClearDepth ? D3D12_CLEAR_FLAG_DEPTH : 0;
		Flags |= bClearStencil ? D3D12_CLEAR_FLAG_STENCIL : 0;

		m_CommandList->ClearDepthStencilView( InView->GetView(), (D3D12_CLEAR_FLAGS)Flags, Depth, Stencil, 0, nullptr );
	}

	void D3D12CommandList::ClearDepthTexture( class RenderTextureBase* InTexture, EDepthStencilViewType Type, bool bClearDepth, float Depth, bool bClearStencil, uint8 Stencil )
	{
		DepthStencilView* View = InTexture->GetDepthStencilView(Type);

		if (bClearDepth)
			drn_check(View->HasDepth());

		if (bClearStencil && View->HasStencil())
			drn_check(View->HasStencil());

		ClearDepthTexture( View, bClearDepth, Depth, bClearStencil, Stencil );
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

	void D3D12CommandList::CopyTextureRegion( RenderTextureBase* SourceTexture, RenderTextureBase* DestTexture, const D3D12_BOX& SourceBox, uint32 DestX, uint32 DestY, uint32 DestZ )
	{
		CD3DX12_TEXTURE_COPY_LOCATION DestCopyLocation(DestTexture->GetResource()->GetResource(), 0);
		CD3DX12_TEXTURE_COPY_LOCATION SourceCopyLocation(SourceTexture->GetResource()->GetResource(), 0);

		ConditionalScopeResourceBarrier ConditionalScopeResourceBarrierDest(this, DestTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, DestCopyLocation.SubresourceIndex);
		ConditionalScopeResourceBarrier ConditionalScopeResourceBarrierSource(this, SourceTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, SourceCopyLocation.SubresourceIndex);

		FlushBarriers();
		m_CommandList->CopyTextureRegion(
			&DestCopyLocation,
			DestX, DestY, DestZ,
			&SourceCopyLocation,
			&SourceBox);

		//CommandListHandle.UpdateResidency(SourceTexture->GetResource());
		//CommandListHandle.UpdateResidency(GetResource());
	}

	void D3D12CommandList::CopySubTextureRegion( RenderTexture2D* SourceTexture, RenderTexture2D* DestTexture, Box2D SourceBox, Box2D DestinationBox )
	{
		const uint32 XOffset = (uint32)( DestinationBox.Min.X );
		const uint32 YOffset = (uint32)( DestinationBox.Min.Y );
		const uint32 Width   = (uint32)( SourceBox.Max.X - SourceBox.Min.X );
		const uint32 Height  = (uint32)( SourceBox.Max.Y - SourceBox.Min.Y );

		const CD3DX12_BOX SourceBoxD3D( (LONG)SourceBox.Min.X, (LONG)SourceBox.Min.Y, (LONG)SourceBox.Max.X, (LONG)SourceBox.Max.Y );

		CD3DX12_TEXTURE_COPY_LOCATION DestCopyLocation( DestTexture->GetResource()->GetResource(), 0 );
		CD3DX12_TEXTURE_COPY_LOCATION SourceCopyLocation( SourceTexture->GetResource()->GetResource(), 0 );

		m_CommandList->CopyTextureRegion(&DestCopyLocation, XOffset, YOffset, 0, &SourceCopyLocation, &SourceBoxD3D);
	}

	void D3D12CommandList::CopyTexture( RenderTextureBase* SourceTexture, RenderTextureBase* DestTexture, const CopyTextureInfo& CopyInfo )
	{
		ConditionalScopeResourceBarrier ConditionalScopeResourceBarrierDest(this, DestTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		ConditionalScopeResourceBarrier ConditionalScopeResourceBarrierSource(this, SourceTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		FlushBarriers();

		const bool bReadback = (DestTexture->GetFlags() & ETextureCreateFlags::CPUReadback) != 0;

		if (CopyInfo.Size != IntVector::Zero || bReadback)
		{
			const IntVector CopySize = CopyInfo.Size == IntVector::Zero ? SourceTexture->GetSizeXYZ() : CopyInfo.Size;

			CD3DX12_BOX SourceBoxD3D(
				CopyInfo.SourcePosition.X,
				CopyInfo.SourcePosition.Y,
				CopyInfo.SourcePosition.Z,
				CopyInfo.SourcePosition.X + CopySize.X,
				CopyInfo.SourcePosition.Y + CopySize.Y,
				CopyInfo.SourcePosition.Z + CopySize.Z
			);

			D3D12_TEXTURE_COPY_LOCATION Src;
			Src.pResource = SourceTexture->GetResource()->GetResource();
			Src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			D3D12_TEXTURE_COPY_LOCATION Dst;
			Dst.pResource = DestTexture->GetResource()->GetResource();
			Dst.Type = bReadback ? D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT : D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			D3D12_RESOURCE_DESC DstDesc = {};
			IntVector TextureSize = DestTexture->GetSizeXYZ();
			DstDesc.Dimension = DestTexture->Is3D() ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D; 
			DstDesc.Width = TextureSize.X;
			DstDesc.Height = TextureSize.Y;
			DstDesc.DepthOrArraySize = TextureSize.Z;
			DstDesc.MipLevels = DestTexture->GetNumMips();
			DstDesc.Format = DestTexture->GetFormat();
			DstDesc.SampleDesc.Count = DestTexture->GetNumSamples();

			for (uint32 SliceIndex = 0; SliceIndex < CopyInfo.NumSlices; ++SliceIndex)
			{
				uint32 SourceSliceIndex = CopyInfo.SourceSliceIndex + SliceIndex;
				uint32 DestSliceIndex   = CopyInfo.DestSliceIndex   + SliceIndex;

				for (uint32 MipIndex = 0; MipIndex < CopyInfo.NumMips; ++MipIndex)
				{
					uint32 SourceMipIndex = CopyInfo.SourceMipIndex + MipIndex;
					uint32 DestMipIndex   = CopyInfo.DestMipIndex   + MipIndex;

					uint32 SizeX = std::max(CopySize.X >> MipIndex, 1);
					uint32 SizeY = std::max(CopySize.Y >> MipIndex, 1);
					uint32 SizeZ = std::max(CopySize.Z >> MipIndex, 1);

					SourceBoxD3D.right  = CopyInfo.SourcePosition.X + SizeX;
					SourceBoxD3D.bottom = CopyInfo.SourcePosition.Y + SizeY;
					SourceBoxD3D.back   = CopyInfo.SourcePosition.Z + SizeZ;

					Src.SubresourceIndex = CalcSubresource(SourceMipIndex, SourceSliceIndex, SourceTexture->GetNumMips());
					Dst.SubresourceIndex = CalcSubresource(DestMipIndex, DestSliceIndex, DestTexture->GetNumMips());

					if (bReadback)
					{
						GetReadBackHeapDescImpl(Dst.PlacedFootprint, GetParentDevice()->GetD3D12Device(), DstDesc, Dst.SubresourceIndex);
					}

					m_CommandList->CopyTextureRegion(
						&Dst, 
						CopyInfo.DestPosition.X,
						CopyInfo.DestPosition.Y,
						CopyInfo.DestPosition.Z,
						&Src,
						&SourceBoxD3D
					);
				}
			}
		}
		else
		{
			m_CommandList->CopyResource(DestTexture->GetResource()->GetResource(), SourceTexture->GetResource()->GetResource());
		}

		//CommandListHandle.UpdateResidency(SourceTexture->GetResource());
		//CommandListHandle.UpdateResidency(DestTexture->GetResource());
		//
		//ConditionalFlushCommandList();

		//DestTexture->SetReadBackListHandle(CommandListHandle);
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

		//ResourceState& ResourceState = GetResourceState(pResource);
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

		ResourceState& State = pResource->GetResourceState();
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

	ResourceState& D3D12CommandList::GetResourceState( class RenderResource* pResource )
	{
		return TrackedResourceState.GetResourceState(pResource);
	}

	void D3D12CommandList::SetIndexBuffer( const ResourceLocation& IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset )
	{
		StateCache.SetIndexBuffer(IndexBufferLocation, Format, Offset);
	}

	void D3D12CommandList::SetStreamSource( uint32 StreamIndex, RenderVertexBuffer* VertexBuffer, uint32 Offset )
	{
		StateCache.SetStreamSource(VertexBuffer ? &VertexBuffer->m_ResourceLocation : nullptr, StreamIndex, Offset);
	}

	void D3D12CommandList::SetGraphicPipelineState( GraphicsPipelineState* InState )
	{
		StateCache.SetGraphicPipelineState(InState);
	}

	void D3D12CommandList::SetStreamStrides( const uint16* Strides )
	{
		StateCache.SetStreamStrides(Strides);
	}

	void D3D12CommandList::SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY Topology )
	{
		StateCache.SetPrimitiveTopology(Topology);
	}

	void D3D12CommandList::SetViewport( float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ )
	{
		drn_check(MinX <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);
		drn_check(MinY <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);
		drn_check(MaxX <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);
		drn_check(MaxY <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);

		D3D12_VIEWPORT Viewport = { MinX, MinY, (MaxX - MinX), (MaxY - MinY), MinZ, MaxZ };
		if (Viewport.Width > 0 && Viewport.Height > 0)
		{
			StateCache.SetViewport(Viewport);
			SetScissorRect(true, MinX, MinY, MaxX, MaxY);
		}
	}

	void D3D12CommandList::SetScissorRect( bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY )
	{
		if (bEnable)
		{
			const CD3DX12_RECT ScissorRect(MinX, MinY, MaxX, MaxY);
			StateCache.SetScissorRect(ScissorRect);
		}
		else
		{
			const D3D12_VIEWPORT& Viewport = StateCache.GetViewport();
			const CD3DX12_RECT ScissorRect((LONG) Viewport.TopLeftX, (LONG) Viewport.TopLeftY, (LONG) Viewport.TopLeftX + (LONG) Viewport.Width, (LONG) Viewport.TopLeftY + (LONG) Viewport.Height);
			StateCache.SetScissorRect(ScissorRect);
		}
	}

	void D3D12CommandList::DrawPrimitive( uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances )
	{
		drn_check(NumPrimitives > 0);
		PrimitiveStats::UpdateStats(std::max(NumInstances, 1U) * NumPrimitives, StateCache.GetGraphicsPipelinePrimitiveTopology());

		NumInstances = std::max<uint32>(1, NumInstances);

		uint32 VertexCount = StateCache.GetVertexCountAndIncrementStat(NumPrimitives);

		//StateCache.ApplyState<D3D12PT_Graphics>();
		StateCache.ApplyState();

		m_CommandList->DrawInstanced(VertexCount, NumInstances, BaseVertexIndex, 0);
	}

	void D3D12CommandList::DrawIndexedPrimitive( RenderIndexBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances )
	{
		drn_check(NumPrimitives > 0);
		PrimitiveStats::UpdateStats(std::max(NumInstances, 1U) * NumPrimitives, StateCache.GetGraphicsPipelinePrimitiveTopology());

		NumInstances = std::max<uint32>(1, NumInstances);

		uint32 IndexCount = StateCache.GetVertexCountAndIncrementStat(NumPrimitives);

		// Verify that we are not trying to read outside the index buffer range
		// test is an optimized version of: StartIndex + IndexCount <= IndexBuffer->GetSize() / IndexBuffer->GetStride() 
		//checkf((StartIndex + IndexCount) * RHIIndexBuffer->GetStride() <= RHIIndexBuffer->GetSize(),
		//	TEXT("Start %u, Count %u, Type %u, Buffer Size %u, Buffer stride %u"), StartIndex, IndexCount, StateCache.GetGraphicsPipelinePrimitiveType(), RHIIndexBuffer->GetSize(), RHIIndexBuffer->GetStride());

		// determine 16bit vs 32bit indices
		const DXGI_FORMAT Format = (IndexBuffer->GetStride() == sizeof(uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
		StateCache.SetIndexBuffer(IndexBuffer->m_ResourceLocation, Format, 0);
		//StateCache.ApplyState<D3D12PT_Graphics>();
		StateCache.ApplyState();

		m_CommandList->DrawIndexedInstanced(IndexCount, NumInstances, StartIndex, BaseVertexIndex, FirstInstance);
	}

	void D3D12CommandList::ClearState()
	{
		StateCache.ClearState();
	}

	void D3D12CommandList::FlushBarriers()
	{
		m_ResourceBarrierBatcher.Flush(m_CommandList.Get());
	}

	void CommandListResourceState::ConditionalInitalize( RenderResource* pResource, ResourceState& ResourceState )
	{
		if (!ResourceState.CheckResourceStateInitalized())
		{
			ResourceState.Initialize(pResource->GetSubresourceCount());
			drn_check(ResourceState.CheckResourceState(D3D12_RESOURCE_STATE_TBD));
		}

		drn_check(ResourceState.CheckResourceStateInitalized());
	}

	ResourceState& CommandListResourceState::GetResourceState( RenderResource* pResource )
	{
		drn_check(pResource->RequiresResourceStateTracking());

		auto it = ResourceStates.insert({pResource, ResourceState()}).first;
		ResourceState& OutResourceState = it->second;

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

	ConditionalScopeResourceBarrier::ConditionalScopeResourceBarrier( D3D12CommandList* InCmdList, RenderResource* pInResource,
		const D3D12_RESOURCE_STATES InDesired, const uint32 InSubresource )
		: CmdList( InCmdList )
		, pResource( pInResource )
		, Current( D3D12_RESOURCE_STATE_TBD )
		, Desired( InDesired )
		, Subresource( InSubresource )
		, bRestoreDefaultState( false )
	{
		if ( !pResource->RequiresResourceStateTracking() )
		{
			Current = pResource->GetDefaultResourceState();
			if ( Current != Desired )
			{
				bRestoreDefaultState = true;
				CmdList->AddTransitionBarrier( pResource, Current, Desired, Subresource );
			}
		}
		else
		{
			CmdList->TransitionResourceWithTracking( pResource, Desired, Subresource );
		}
	}

	ConditionalScopeResourceBarrier::~ConditionalScopeResourceBarrier()
	{
		if ( bRestoreDefaultState )
		{
			CmdList->AddTransitionBarrier( pResource, Desired, Current, Subresource );
		}
	}

// -----------------------------------------------------------------------------------------

	std::atomic<uint64> PrimitiveStats::Stats[(uint8)EPrimitiveStatGroups::Max];

	void PrimitiveStats::UpdateStats( uint64 Count, D3D_PRIMITIVE_TOPOLOGY Topology )
	{
#if RENDER_STATS
		int32 Index = -1;
		switch ( Topology )
		{
		case(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST):	Index = (uint8)EPrimitiveStatGroups::Triangle; break;
		case(D3D_PRIMITIVE_TOPOLOGY_LINELIST):		Index = (uint8)EPrimitiveStatGroups::Line; break;

		default: drn_check(false)
		}

		if (Index >= 0)
			Stats[Index].fetch_add(Count);
#endif
	}

	void PrimitiveStats::UpdateStats( uint64 Count, EPrimitiveStatGroups Group )
	{
#if RENDER_STATS
		Stats[(uint8)Group].fetch_add(Count);
#endif
	}

	void PrimitiveStats::ClearStats()
	{
#if RENDER_STATS
		for (int32 i = 0; i < (uint8)EPrimitiveStatGroups::Max; i++)
		{
			Stats[i].store(0);
		}
#endif
	}

	std::string PrimitiveStats::GetStatName( EPrimitiveStatGroups Group )
	{
		switch ( Group )
		{
		case PrimitiveStats::EPrimitiveStatGroups::Triangle:	return "Triangle";
		case PrimitiveStats::EPrimitiveStatGroups::Line:		return "Line";
		default: drn_check(false); return "invalid";
		}
	}

	int32 PrimitiveStats::GetStatSize( EPrimitiveStatGroups Group )
	{
		return Stats[(uint8)Group].load();
	}

}  // namespace Drn