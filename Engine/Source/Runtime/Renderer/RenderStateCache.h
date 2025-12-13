#pragma once

#include "ForwardTypes.h"

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

	class RenderStateCache : public DeviceChild
	{
	public:
		RenderStateCache()
			: DeviceChild(nullptr)
		{}

		virtual ~RenderStateCache()
		{}

		void Init(Device* InParent, D3D12CommandList* InCmdContext, const RenderStateCache* AncestralState);
		virtual void ClearState();
		void InheritState(const RenderStateCache& AncestralCache);
		void DirtyState();

		void SetIndexBuffer(const ResourceLocation& IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset);

	private:

		struct
		{
			struct
			{
				IndexBufferCache IBCache;
			} Graphics;

			struct
			{
			} Compute;

			struct
			{
			} Common;
		} PipelineState;

		D3D12CommandList* CmdList;
	};
}