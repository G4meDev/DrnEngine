#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderResource.h"

namespace Drn
{
	class Device;
	class D3D12CommandList;

	class RenderQuery : public SimpleRenderResource, public DeviceChild
	{
	public:

		TRefCountPtr<RenderResource> ResultBuffer;

		uint64 Result;
		uint32 Timestamp;
		uint32 HeapIndex;
		uint32 FrameSubmitted;
		const ERenderQueryType Type;
		bool bResultIsCached : 1;
		bool bResolved : 1;

		RenderQuery(Device* Parent, ERenderQueryType InQueryType) :
			DeviceChild(Parent),
			ResultBuffer(nullptr),
			Result(0),
			Timestamp(0),
			Type(InQueryType)
		{
			Reset();
		}

		inline void Reset()
		{
			ResultBuffer = nullptr;
			HeapIndex = -1;
			bResultIsCached = false;
			bResolved = false;
			FrameSubmitted = -1;
		}

		inline void MarkResolved(D3D12CommandList* CommandList, RenderResource* InResultBuffer);

		uint64 SumbmittedFence;
	private:
	};

	class RenderQueryHeap : public DeviceChild
	{
	private:
		struct QueryBatch
		{
		public:
			uint32 StartElement;
			uint32 ElementCount;
			bool bOpen;

			TRefCountPtr<ID3D12QueryHeap> UsedQueryHeap;
			TRefCountPtr<RenderResource> UsedResultBuffer;
		
			std::vector<RenderQuery*> RenderQueries;

			QueryBatch()
			{
				RenderQueries.reserve(256);
				Clear();
			}

			inline void Clear()
			{
				StartElement = 0;
				ElementCount = 0;
				bOpen = false;
				RenderQueries.clear();

				UsedQueryHeap = nullptr;
				UsedResultBuffer = nullptr;
			}
		};

	public:
		RenderQueryHeap(Device* InParent, const D3D12_QUERY_TYPE InQueryType, uint32 InQueryHeapCount, uint32 InMaxActiveBatches);

		void Init();
		void Destroy();

		void StartQueryBatch( D3D12CommandList* CmdList, uint32 NumQueriesInBatch );
		void EndQueryBatchAndResolveQueryData(D3D12CommandList* CmdList);

		uint32 AllocQuery(D3D12CommandList* CmdList);
		void BeginQuery(D3D12CommandList* CmdList, RenderQuery* Query);
		void EndQuery(D3D12CommandList* CmdList, RenderQuery* Query);

	private:
		uint32 GetNextElement(uint32 InElement);
		uint32 GetNextBatchElement(uint32 InBatchElement);

		void CreateQueryHeap();
		void DestroyQueryHeap();

		uint64 GetResultBufferOffsetForElement(uint32 InElement) const { return ResultSize * InElement; };

	private:
		QueryBatch CurrentQueryBatch;

		std::vector<QueryBatch> ActiveQueryBatches;
		uint32 LastBatch;

		uint32 ActiveAllocatedElementCount;

		uint32 LastAllocatedElement;
		const D3D12_QUERY_TYPE QueryType;
		uint32 QueryHeapCount;
		TRefCountPtr<ID3D12QueryHeap> ActiveQueryHeap;
		TRefCountPtr<RenderResource> ActiveResultBuffer;

		static const uint32 ResultSize = 8;
	};

	class RenderQueryPool;

	class PooledRenderQuery
	{
		TRefCountPtr<RenderQuery> Query;
		RenderQueryPool* QueryPool = nullptr;
	
	public:
		PooledRenderQuery() = default;
		PooledRenderQuery(RenderQueryPool* InQueryPool, TRefCountPtr<RenderQuery>&& InQuery);
		~PooledRenderQuery();
	
		PooledRenderQuery(const PooledRenderQuery&) = delete;
		PooledRenderQuery& operator=(const PooledRenderQuery&) = delete;
		PooledRenderQuery(PooledRenderQuery&&) = default;
		PooledRenderQuery& operator=(PooledRenderQuery&&) = default;
	
		bool IsValid() const
		{
			return Query.IsValid();
		}
	
		RenderQuery* GetQuery() const
		{
			return Query;
		}
	
		void ReleaseQuery();
	};

	class RenderQueryPool : public SimpleRenderResource, public DeviceChild
	{
	public:
		RenderQueryPool(Device* InParent, ERenderQueryType InQueryType, uint32 InNumQueries);
		virtual ~RenderQueryPool();
		PooledRenderQuery AllocateQuery();

	private:
		friend class PooledRenderQuery;
		void ReleaseQuery(TRefCountPtr<RenderQuery>&& Query);

		ERenderQueryType QueryType;
			uint32 NumQueries = 0;
		uint32 AllocatedQueries = 0;
		std::vector<TRefCountPtr<RenderQuery>> Queries;
	};
}