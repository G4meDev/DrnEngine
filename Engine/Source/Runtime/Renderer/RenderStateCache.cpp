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

        }  // namespace Drn