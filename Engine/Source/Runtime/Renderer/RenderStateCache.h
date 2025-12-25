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

	struct RootConstantsCache
	{
		RootConstantsCache()
		{
			Clear();
		};

		inline void Clear()
		{
			memset(RootConstants, UINT32_MAX, sizeof(RootConstants));
			MaxBoundConstant = -1;
			BoundConstantsMask = 0;
		}

		uint32 RootConstants[NUM_ROOT_CONSTANTS];
		int32 MaxBoundConstant;
		uint32 BoundConstantsMask;
	};

	class RenderStateCache : public DeviceChild
	{
	public:
		RenderStateCache()
			: DeviceChild(nullptr)
			, bNeedSetVB(false)
			, bNeedSetPrimitiveTopology(false)
			, bNeedSetScissorRects(false)
			, bNeedSetViewports(false)
		{}

		virtual ~RenderStateCache()
		{}

		void Init(Device* InParent, D3D12CommandList* InCmdContext, const RenderStateCache* AncestralState);
		virtual void ClearState();
		void InheritState(const RenderStateCache& AncestralCache);
		void DirtyState();

		void SetGraphicRootConstant(uint32 Value, int32 Index);

		void SetIndexBuffer(const ResourceLocation& IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset);
		void SetStreamSource(ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset);
		void SetStreamSource(ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Offset);

		void SetStreamStrides(const uint16* Strides);

		static void ValidateScissorRect(const D3D12_VIEWPORT& Viewport, const D3D12_RECT& ScissorRect);

		void SetScissorRects(uint32 Count, const D3D12_RECT* const ScissorRects);
		void SetScissorRect(const D3D12_RECT& ScissorRect);

		void SetViewport(const D3D12_VIEWPORT& Viewport);
		void SetViewports(uint32 Count, const D3D12_VIEWPORT* const Viewports);

		void ApplyState();

		inline uint32 GetVertexCountAndIncrementStat(uint32 NumPrimitives)
		{
			//*PipelineState.Graphics.CurrentPrimitiveStat += NumPrimitives;
			return PipelineState.Graphics.PrimitiveTypeFactor * NumPrimitives + PipelineState.Graphics.PrimitiveTypeOffset;
		}

		inline uint32 GetNumTrianglesStat() const { return PipelineState.Graphics.NumTriangles; }
		inline uint32 GetNumLinesStat() const { return PipelineState.Graphics.NumLines; }

		inline const D3D12_VIEWPORT& GetViewport(int32 Index = 0) const
		{
			return PipelineState.Graphics.CurrentViewport[Index];
		}

		inline EPrimitiveType GetGraphicsPipelinePrimitiveTopology() const
		{
			return PipelineState.Graphics.CurrentPrimitiveType;
		}

		void SetGraphicPipelineState(class GraphicsPipelineState* InState);

	private:

		void InternalSetGraphicPipelineState();
		void InternalSetStreamSource(ResourceLocation* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset);
		uint32 GetPrimitiveTopologyFactor(D3D_PRIMITIVE_TOPOLOGY InTopology);
		uint32 GetPrimitiveTopologyOffset(D3D_PRIMITIVE_TOPOLOGY InTopology);
		uint32* GetPrimitiveTopologyStat(D3D_PRIMITIVE_TOPOLOGY InTopology);

		class ID3D12RootSignature* GetGraphicsRootSignature() const;

		static inline D3D_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveType(EPrimitiveType PrimitiveType, bool bUsingTessellation)
		{
			static const uint8 D3D12PrimitiveType[] =
			{
				D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
				D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
				D3D_PRIMITIVE_TOPOLOGY_LINELIST,
				D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
			};

			if (bUsingTessellation)
			{
				if (PrimitiveType == EPrimitiveType::TriangleList)
				{
					return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
				}
				else
				{
					drn_check(false);
					return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
				}
			}

			D3D_PRIMITIVE_TOPOLOGY D3DType = (D3D_PRIMITIVE_TOPOLOGY) D3D12PrimitiveType[(uint8)PrimitiveType];
			return D3DType;
		}

		struct
		{
			struct
			{
				bool bNeedSetRootSignature;
				RootConstantsCache RCCache;

				uint32	CurrentNumberOfViewports;
				D3D12_VIEWPORT CurrentViewport[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

				D3D12_RECT CurrentScissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				uint32 CurrentNumberOfScissorRects;

				IndexBufferCache IBCache;
				VertexBufferCache VBCache;

				EPrimitiveType CurrentPrimitiveType;
				D3D_PRIMITIVE_TOPOLOGY CurrentPrimitiveTopology;
				uint32 PrimitiveTypeFactor;
				uint32 PrimitiveTypeOffset;
				uint32* CurrentPrimitiveStat;
				uint32 NumTriangles;
				uint32 NumLines;

				TRefCountPtr<class GraphicsPipelineState> CurrentPipelineStateObject;
				uint16 StreamStrides[MAX_VERTEX_ELEMENT_COUT];
			} Graphics;

			struct
			{
			} Compute;

			struct
			{
				bool bNeedSetPSO;
				ID3D12PipelineState* CurrentPipelineStateObject;
			} Common;
		} PipelineState;

		bool bNeedSetVB;
		bool bNeedSetViewports;
		bool bNeedSetScissorRects;
		bool bNeedSetPrimitiveTopology;

		D3D12CommandList* CmdList;
	};
}