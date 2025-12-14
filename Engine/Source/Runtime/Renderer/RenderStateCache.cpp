#include "DrnPCH.h"
#include "RenderStateCache.h"

namespace Drn
{
	void RenderStateCache::Init( Device* InParent, D3D12CommandList* InCmdContext, const RenderStateCache* AncestralState )
	{
		Parent = InParent;
		CmdList = InCmdContext;

		if (AncestralState)
		{
			InheritState(*AncestralState);
		}
		else
		{
			ClearState();
		}
	}

	void RenderStateCache::ClearState()
	{
		PipelineState.Graphics.IBCache.Clear();
		PipelineState.Graphics.VBCache.Clear();

		// TODO: remove
		PipelineState.Graphics.CurrentPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		PipelineState.Graphics.CurrentPrimitiveStat = nullptr;
		PipelineState.Graphics.PrimitiveTypeFactor = 3;
		PipelineState.Graphics.PrimitiveTypeOffset = 0;
	}

	void RenderStateCache::InheritState( const RenderStateCache& AncestralCache )
	{
		memcpy( &PipelineState, &AncestralCache.PipelineState, sizeof( PipelineState ) );
		DirtyState();
	}

	void RenderStateCache::DirtyState()
	{
		
	}

	void RenderStateCache::SetIndexBuffer( const ResourceLocation& IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset )
	{
		D3D12_GPU_VIRTUAL_ADDRESS BufferLocation = IndexBufferLocation.GetGPUVirtualAddress() + Offset;
		uint32 SizeInBytes = IndexBufferLocation.GetSize() - Offset;

		D3D12_INDEX_BUFFER_VIEW& CurrentView = PipelineState.Graphics.IBCache.CurrentIndexBufferView;

		if (BufferLocation != CurrentView.BufferLocation || SizeInBytes != CurrentView.SizeInBytes || Format != CurrentView.Format)
		{
			CurrentView.BufferLocation = BufferLocation;
			CurrentView.SizeInBytes = SizeInBytes;
			CurrentView.Format = Format;

			if (IndexBufferLocation.GetResource()->RequiresResourceStateTracking())
			{
				CmdList->TransitionResourceWithTracking(IndexBufferLocation.GetResource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
			}

			CmdList->GetD3D12CommandList()->IASetIndexBuffer(&CurrentView);
		}
	}

	void RenderStateCache::SetStreamSource( ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset )
	{
		//drn_check(Stride == PipelineState.Graphics.StreamStrides[StreamIndex]);
		InternalSetStreamSource(VertexBufferLocation, StreamIndex, Stride, Offset);
	}

	void RenderStateCache::SetStreamSource( ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Offset )
	{
		// TODO: add stride from input layout
		const uint32 Stride = sizeof(float) * 3; // position
		InternalSetStreamSource(VertexBufferLocation, StreamIndex, Stride, Offset);
	}

	void RenderStateCache::InternalSetStreamSource( ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset )
	{
		drn_check(VertexBufferLocation == nullptr || VertexBufferLocation->GetResource());
		drn_check(StreamIndex < ARRAYSIZE(PipelineState.Graphics.VBCache.CurrentVertexBufferResources));

		__declspec(align(16)) D3D12_VERTEX_BUFFER_VIEW NewView;
		NewView.BufferLocation = (VertexBufferLocation) ? VertexBufferLocation->GetGPUVirtualAddress() + Offset : 0;
		NewView.StrideInBytes = Stride;
		NewView.SizeInBytes = (VertexBufferLocation) ? VertexBufferLocation->GetSize() - Offset : 0;

		D3D12_VERTEX_BUFFER_VIEW& CurrentView = PipelineState.Graphics.VBCache.CurrentVertexBufferViews[StreamIndex];

		if (NewView.BufferLocation != CurrentView.BufferLocation ||
			NewView.StrideInBytes != CurrentView.StrideInBytes ||
			NewView.SizeInBytes != CurrentView.SizeInBytes)
		{
			bNeedSetVB = true;
			PipelineState.Graphics.VBCache.CurrentVertexBufferResources[StreamIndex] = VertexBufferLocation;

			if (VertexBufferLocation != nullptr)
			{
				//PipelineState.Graphics.VBCache.ResidencyHandles[StreamIndex] = VertexBufferLocation->GetResource()->GetResidencyHandle();
				memcpy(&CurrentView, &NewView, sizeof(D3D12_VERTEX_BUFFER_VIEW));
				PipelineState.Graphics.VBCache.BoundVBMask |= ((uint32)1 << StreamIndex);
			}
			else
			{
				memset(&CurrentView, 0, sizeof(CurrentView));
				PipelineState.Graphics.VBCache.CurrentVertexBufferResources[StreamIndex] = nullptr;
				//PipelineState.Graphics.VBCache.ResidencyHandles[StreamIndex] = nullptr;

				PipelineState.Graphics.VBCache.BoundVBMask &= ~((uint32)1 << StreamIndex);
			}

			if (PipelineState.Graphics.VBCache.BoundVBMask)
			{
				PipelineState.Graphics.VBCache.MaxBoundVertexBufferIndex = std::_Floor_of_log_2(PipelineState.Graphics.VBCache.BoundVBMask);
			}
			else
			{
				PipelineState.Graphics.VBCache.MaxBoundVertexBufferIndex = -1;
			}
		}

		if (VertexBufferLocation)
		{
			RenderResource* const pResource = VertexBufferLocation->GetResource();
			if (pResource && pResource->RequiresResourceStateTracking())
			{
				drn_check(pResource->GetSubresourceCount() == 1);
				CmdList->TransitionResourceWithTracking(pResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
		}
	}

	void RenderStateCache::ApplyState()
	{
		if (bNeedSetVB)
		{
			VertexBufferCache& Cache = PipelineState.Graphics.VBCache;

			const uint32 Count = Cache.MaxBoundVertexBufferIndex + 1;
			if (Count)
			{
				CmdList->GetD3D12CommandList()->IASetVertexBuffers(0, Count, Cache.CurrentVertexBufferViews);
			}
		}

		CmdList->FlushBarriers();
	}



        }  // namespace Drn