#pragma once

#include "ForwardTypes.h"

#define MAX_VBS D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT

namespace Drn
{
	struct IndexBufferCache
	{
		IndexBufferCache()
		{
			Clear();
		}

		inline void Clear()
		{
			memset(&CurrentIndexBufferView, 0, sizeof(CurrentIndexBufferView));
		}

		D3D12_INDEX_BUFFER_VIEW CurrentIndexBufferView;
	};

	struct VertexBufferCache
	{
		VertexBufferCache()
		{
			Clear();
		};

		inline void Clear()
		{
			memset(CurrentVertexBufferViews, 0, sizeof(CurrentVertexBufferViews));
			memset(CurrentVertexBufferResources, 0, sizeof(CurrentVertexBufferResources));
			MaxBoundVertexBufferIndex = -1;
			BoundVBMask = 0;
		}

		D3D12_VERTEX_BUFFER_VIEW CurrentVertexBufferViews[MAX_VBS];
		ResourceLocation* CurrentVertexBufferResources[MAX_VBS];
		int32 MaxBoundVertexBufferIndex;
		uint32 BoundVBMask;
	};

	class RenderStateCache : public DeviceChild
	{
	public:
		RenderStateCache()
			: DeviceChild(nullptr)
			, bNeedSetVB(false)
		{}

		virtual ~RenderStateCache()
		{}

		void Init(Device* InParent, D3D12CommandList* InCmdContext, const RenderStateCache* AncestralState);
		virtual void ClearState();
		void InheritState(const RenderStateCache& AncestralCache);
		void DirtyState();

		void SetIndexBuffer(const ResourceLocation& IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset);
		void SetStreamSource(ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset);
		void SetStreamSource(ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Offset);

		void ApplyState();

		inline uint32 GetVertexCountAndIncrementStat(uint32 NumPrimitives)
		{
			//*PipelineState.Graphics.CurrentPrimitiveStat += NumPrimitives;
			return PipelineState.Graphics.PrimitiveTypeFactor * NumPrimitives + PipelineState.Graphics.PrimitiveTypeOffset;
		}

	private:

		void InternalSetStreamSource(ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset);

		struct
		{
			struct
			{
				IndexBufferCache IBCache;
				VertexBufferCache VBCache;

				D3D_PRIMITIVE_TOPOLOGY CurrentPrimitiveTopology;
				uint32 PrimitiveTypeFactor;
				uint32 PrimitiveTypeOffset;
				uint32* CurrentPrimitiveStat;
			} Graphics;

			struct
			{
			} Compute;

			struct
			{
			} Common;
		} PipelineState;

		bool bNeedSetVB;

		D3D12CommandList* CmdList;
	};
}