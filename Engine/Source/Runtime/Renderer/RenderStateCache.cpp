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

		PipelineState.Graphics.CurrentPrimitiveType = EPrimitiveType::Max;
		PipelineState.Graphics.CurrentPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

		memset(PipelineState.Graphics.CurrentViewport, 0, sizeof(PipelineState.Graphics.CurrentViewport));
		PipelineState.Graphics.CurrentNumberOfViewports = 0;

		PipelineState.Graphics.CurrentNumberOfScissorRects = 0;

		PipelineState.Graphics.CurrentPipelineStateObject = nullptr;
		//PipelineState.Compute.CurrentPipelineStateObject = nullptr;
		PipelineState.Common.CurrentPipelineStateObject = nullptr;
	}

	void RenderStateCache::InheritState( const RenderStateCache& AncestralCache )
	{
		memcpy( &PipelineState, &AncestralCache.PipelineState, sizeof( PipelineState ) );
		DirtyState();
	}

	void RenderStateCache::DirtyState()
	{
		bNeedSetVB = true;
		bNeedSetViewports = true;
		bNeedSetScissorRects = true;
		bNeedSetPrimitiveTopology = true;
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
		drn_check(Stride == PipelineState.Graphics.StreamStrides[StreamIndex]);
		InternalSetStreamSource(VertexBufferLocation, StreamIndex, Stride, Offset);
	}

	void RenderStateCache::SetStreamSource( ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Offset )
	{
		InternalSetStreamSource(VertexBufferLocation, StreamIndex, PipelineState.Graphics.StreamStrides[StreamIndex], Offset);
	}

	void RenderStateCache::SetStreamStrides(const uint16* Strides)
	{
		memcpy(PipelineState.Graphics.StreamStrides, Strides, sizeof(PipelineState.Graphics.StreamStrides));
	}

	void RenderStateCache::ValidateScissorRect( const D3D12_VIEWPORT& Viewport, const D3D12_RECT& ScissorRect )
	{
		drn_check(ScissorRect.left   >= (LONG)Viewport.TopLeftX);
		drn_check(ScissorRect.top    >= (LONG)Viewport.TopLeftY);
		drn_check(ScissorRect.right  <= (LONG)Viewport.TopLeftX + (LONG)Viewport.Width);
		drn_check(ScissorRect.bottom <= (LONG)Viewport.TopLeftY + (LONG)Viewport.Height);
		drn_check(ScissorRect.left <= ScissorRect.right && ScissorRect.top <= ScissorRect.bottom);
	}

	void RenderStateCache::SetScissorRect( const D3D12_RECT& ScissorRect )
	{
		ValidateScissorRect(PipelineState.Graphics.CurrentViewport[0], ScissorRect);

		if ((PipelineState.Graphics.CurrentNumberOfScissorRects != 1 || memcmp(&PipelineState.Graphics.CurrentScissorRects[0], &ScissorRect, sizeof(D3D12_RECT))))
		{
			memcpy(&PipelineState.Graphics.CurrentScissorRects[0], &ScissorRect, sizeof(D3D12_RECT));
			PipelineState.Graphics.CurrentNumberOfScissorRects = 1;
			bNeedSetScissorRects = true;
		}
	}

	void RenderStateCache::SetScissorRects( uint32 Count, const D3D12_RECT* const ScissorRects )
	{
		drn_check(Count < ARRAYSIZE(PipelineState.Graphics.CurrentScissorRects));

		for (uint32 Rect = 0; Rect < Count; ++Rect)
		{
			ValidateScissorRect(PipelineState.Graphics.CurrentViewport[Rect], ScissorRects[Rect]);
		}

		if ((PipelineState.Graphics.CurrentNumberOfScissorRects != Count || memcmp(&PipelineState.Graphics.CurrentScissorRects[0], ScissorRects, sizeof(D3D12_RECT) * Count)))
		{
			memcpy(&PipelineState.Graphics.CurrentScissorRects[0], ScissorRects, sizeof(D3D12_RECT) * Count);
			PipelineState.Graphics.CurrentNumberOfScissorRects = Count;
			bNeedSetScissorRects = true;
		}
	}

	void RenderStateCache::SetViewport( const D3D12_VIEWPORT& Viewport )
	{
		if ((PipelineState.Graphics.CurrentNumberOfViewports != 1 || memcmp(&PipelineState.Graphics.CurrentViewport[0], &Viewport, sizeof(D3D12_VIEWPORT))))
		{
			memcpy(&PipelineState.Graphics.CurrentViewport[0], &Viewport, sizeof(D3D12_VIEWPORT));
			PipelineState.Graphics.CurrentNumberOfViewports = 1;
			bNeedSetViewports = true;
		}
	}

	void RenderStateCache::SetViewports( uint32 Count, const D3D12_VIEWPORT* const Viewports )
	{
		drn_check(Count < ARRAYSIZE(PipelineState.Graphics.CurrentViewport));
		if ((PipelineState.Graphics.CurrentNumberOfViewports != Count || memcmp(&PipelineState.Graphics.CurrentViewport[0], Viewports, sizeof(D3D12_VIEWPORT) * Count)))
		{
			memcpy(&PipelineState.Graphics.CurrentViewport[0], Viewports, sizeof(D3D12_VIEWPORT) * Count);
			PipelineState.Graphics.CurrentNumberOfViewports = Count;
			bNeedSetViewports = true;
		}
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

	uint32 RenderStateCache::GetPrimitiveTopologyFactor( D3D_PRIMITIVE_TOPOLOGY InTopology )
	{
		switch ( InTopology )
		{
		case(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST):	return 3;
		case(D3D_PRIMITIVE_TOPOLOGY_LINELIST):		return 2;

		default: drn_check(false); return 0;
		}
	}

	uint32 RenderStateCache::GetPrimitiveTopologyOffset( D3D_PRIMITIVE_TOPOLOGY InTopology )
	{
		switch ( InTopology )
		{
		case(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST):	return 0;
		case(D3D_PRIMITIVE_TOPOLOGY_LINELIST):		return 0;

		default: drn_check(false); return 0;
		}
	}

	uint32* RenderStateCache::GetPrimitiveTopologyStat( D3D_PRIMITIVE_TOPOLOGY InTopology )
	{
		return InTopology == D3D_PRIMITIVE_TOPOLOGY_LINELIST ? &PipelineState.Graphics.NumLines : &PipelineState.Graphics.NumTriangles;
	}

	ID3D12RootSignature* RenderStateCache::GetGraphicsRootSignature() const 
	{
		return PipelineState.Graphics.CurrentPipelineStateObject ? PipelineState.Graphics.CurrentPipelineStateObject->RootSignature : nullptr;
	}

	void RenderStateCache::ApplyState()
	{
		ID3D12RootSignature* pRootSignature = GetGraphicsRootSignature();

		//if (PipelineState.Graphics.bNeedSetRootSignature)
		//{
		//	CmdList->GetD3D12CommandList()->SetGraphicsRootSignature(pRootSignature);
		//	PipelineState.Graphics.bNeedSetRootSignature = false;
		//}

		//InternalSetGraphicPipelineState();

		if (bNeedSetVB)
		{
			VertexBufferCache& Cache = PipelineState.Graphics.VBCache;

			const uint32 Count = Cache.MaxBoundVertexBufferIndex + 1;
			if (Count)
			{
				CmdList->GetD3D12CommandList()->IASetVertexBuffers(0, Count, Cache.CurrentVertexBufferViews);
			}
		}

		if (bNeedSetViewports)
		{
			bNeedSetViewports = false;
			CmdList->GetD3D12CommandList()->RSSetViewports(PipelineState.Graphics.CurrentNumberOfViewports, PipelineState.Graphics.CurrentViewport);
		}
		if (bNeedSetScissorRects)
		{
			bNeedSetScissorRects = false;
			CmdList->GetD3D12CommandList()->RSSetScissorRects(PipelineState.Graphics.CurrentNumberOfScissorRects, PipelineState.Graphics.CurrentScissorRects);
		}
		if (bNeedSetPrimitiveTopology)
		{
			bNeedSetPrimitiveTopology = false;
			CmdList->GetD3D12CommandList()->IASetPrimitiveTopology(PipelineState.Graphics.CurrentPrimitiveTopology);
		}

		CmdList->FlushBarriers();
	}

	void RenderStateCache::InternalSetGraphicPipelineState()
	{
		bool bNeedSetPSO = PipelineState.Common.bNeedSetPSO;
		if (PipelineState.Common.CurrentPipelineStateObject != PipelineState.Graphics.CurrentPipelineStateObject->PipelineState)
		{
			PipelineState.Common.CurrentPipelineStateObject = PipelineState.Graphics.CurrentPipelineStateObject->PipelineState;
			bNeedSetPSO = true;
		}

		if (bNeedSetPSO)
		{
			drn_check(PipelineState.Common.CurrentPipelineStateObject);
			CmdList->GetD3D12CommandList()->SetPipelineState(PipelineState.Common.CurrentPipelineStateObject);
			PipelineState.Common.bNeedSetPSO = false;
		}
	}

	void RenderStateCache::SetGraphicPipelineState( GraphicsPipelineState* InState )
	{
		drn_check(InState);

		//if (PipelineState.Graphics.CurrentPipelineStateObject != InState)
		if (true)
		{
			SetStreamStrides(InState->StreamStrides);
			//SetShader(InState->GetVertexShader());
			//SetShader(InState->GetPixelShader());
			//SetShader(InState->GetDomainShader());
			//SetShader(InState->GetHullShader());
			//SetShader(InState->GetGeometryShader());

			if ( GetGraphicsRootSignature() != InState->RootSignature)
			{
				PipelineState.Graphics.bNeedSetRootSignature = true;
			}

			PipelineState.Common.bNeedSetPSO = true;
			PipelineState.Graphics.CurrentPipelineStateObject = InState;

			EPrimitiveType PrimitiveType = InState->PipelineStateInitializer.PrimitiveType;
			if (PipelineState.Graphics.CurrentPrimitiveType != PrimitiveType)
			{
				const bool bUsingTessellation = InState->GetHullShader() && InState->GetDomainShader();
				PipelineState.Graphics.CurrentPrimitiveType = PrimitiveType;
				PipelineState.Graphics.CurrentPrimitiveTopology = GetD3D12PrimitiveType(PrimitiveType, bUsingTessellation);
				bNeedSetPrimitiveTopology = true;

				PipelineState.Graphics.PrimitiveTypeFactor = (PrimitiveType == EPrimitiveType::TriangleList) ? 3 : (PrimitiveType == EPrimitiveType::LineList) ? 2 : 1;
				PipelineState.Graphics.PrimitiveTypeOffset = (PrimitiveType == EPrimitiveType::TriangleStrip) ? 2 : 0;
				PipelineState.Graphics.CurrentPrimitiveStat = (PrimitiveType == EPrimitiveType::LineList) ? &PipelineState.Graphics.NumLines : &PipelineState.Graphics.NumTriangles;
			}

			InternalSetGraphicPipelineState();
		}
	}

        }  // namespace Drn