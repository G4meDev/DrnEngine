#include "DrnPCH.h"
#include "RenderQuery.h"

namespace Drn
{
	RenderQueryHeap::RenderQueryHeap( Device* InParent, const D3D12_QUERY_TYPE InQueryType, uint32 InQueryHeapCount, uint32 InMaxActiveBatches )
		: DeviceChild(InParent)
		, LastBatch(InMaxActiveBatches - 1)
		, ActiveAllocatedElementCount(0)
		, LastAllocatedElement(InQueryHeapCount - 1)
		, QueryType(InQueryType)
		, QueryHeapCount(InQueryHeapCount)
		, ActiveQueryHeap(nullptr)
		, ActiveResultBuffer(nullptr)
	{
		drn_check(QueryType == D3D12_QUERY_TYPE_OCCLUSION || QueryType == D3D12_QUERY_TYPE_TIMESTAMP);
	
		CurrentQueryBatch.Clear();
		ActiveQueryBatches.resize(InMaxActiveBatches);
	}

	void RenderQueryHeap::Init()
	{
		drn_check(GetParentDevice());
		drn_check(GetParentDevice()->GetD3D12Device());

		CreateQueryHeap();
	}

	void RenderQueryHeap::Destroy()
	{
		DestroyQueryHeap();
	}

	void RenderQueryHeap::StartQueryBatch( D3D12CommandList* CmdList, uint32 NumQueriesInBatch )
	{
		drn_check(!CurrentQueryBatch.bOpen);

		CurrentQueryBatch.Clear();

		if (ActiveAllocatedElementCount + NumQueriesInBatch > QueryHeapCount)
		{
			DestroyQueryHeap();

			QueryHeapCount = Align(NumQueriesInBatch + QueryHeapCount, 65536 / ResultSize);

			CreateQueryHeap();

			ActiveAllocatedElementCount = 0;
			LastAllocatedElement = QueryHeapCount - 1;
		}

		// Start a new batch
		CurrentQueryBatch.StartElement = GetNextElement(LastAllocatedElement);
		CurrentQueryBatch.UsedQueryHeap = ActiveQueryHeap;
		CurrentQueryBatch.UsedResultBuffer = ActiveResultBuffer;
		CurrentQueryBatch.bOpen = true;
	}

	void RenderQueryHeap::EndQueryBatchAndResolveQueryData( D3D12CommandList* CmdList )
	{
		drn_check(CmdList);

		if (!CurrentQueryBatch.bOpen)
		{
			return;
		}

		drn_check(CurrentQueryBatch.bOpen);

		CurrentQueryBatch.bOpen = false;
		if (CurrentQueryBatch.ElementCount == 0)
		{
			return;
		}

		ActiveAllocatedElementCount += CurrentQueryBatch.ElementCount;
		//checkf(ActiveAllocatedElementCount <= QueryHeapCount, TEXT("The query heap is too small. Either increase the heap count (larger resource) or decrease MAX_ACTIVE_BATCHES."));
		drn_check(ActiveAllocatedElementCount <= QueryHeapCount);

		LastBatch = GetNextBatchElement(LastBatch);
		ActiveQueryBatches[LastBatch] = CurrentQueryBatch;
	
		QueryBatch& OldestBatch = ActiveQueryBatches[GetNextBatchElement(LastBatch)];
		if (OldestBatch.UsedQueryHeap == ActiveQueryHeap)
		{
			drn_check(ActiveAllocatedElementCount >= OldestBatch.ElementCount);
			ActiveAllocatedElementCount -= OldestBatch.ElementCount;
		}

		if (CurrentQueryBatch.StartElement + CurrentQueryBatch.ElementCount <= QueryHeapCount)
		{
			CmdList->GetD3D12CommandList()->ResolveQueryData(
				ActiveQueryHeap, QueryType, CurrentQueryBatch.StartElement, CurrentQueryBatch.ElementCount,
				ActiveResultBuffer->GetResource(), GetResultBufferOffsetForElement(CurrentQueryBatch.StartElement));
		}
		else
		{
			CmdList->GetD3D12CommandList()->ResolveQueryData(
				ActiveQueryHeap, QueryType, CurrentQueryBatch.StartElement, QueryHeapCount - CurrentQueryBatch.StartElement,
				ActiveResultBuffer->GetResource(), GetResultBufferOffsetForElement(CurrentQueryBatch.StartElement));
			CmdList->GetD3D12CommandList()->ResolveQueryData(
				ActiveQueryHeap, QueryType, 0, CurrentQueryBatch.ElementCount - (QueryHeapCount - CurrentQueryBatch.StartElement),
				ActiveResultBuffer->GetResource(), 0);
		}

		drn_check(CurrentQueryBatch.UsedResultBuffer == ActiveResultBuffer);
		for (int32 i = 0; i < CurrentQueryBatch.RenderQueries.size(); i++)
		{
			CurrentQueryBatch.RenderQueries[i]->MarkResolved(CmdList, ActiveResultBuffer);
		}
	}

	uint32 RenderQueryHeap::AllocQuery( D3D12CommandList* CmdList )
	{
		drn_check(CmdList);

		const uint32 CurrentElement = GetNextElement(LastAllocatedElement);

		if (QueryType == D3D12_QUERY_TYPE_OCCLUSION)
		{
			drn_check(CurrentQueryBatch.bOpen);
		}
		else
		{
			if (!CurrentQueryBatch.bOpen)
			{
				StartQueryBatch(CmdList, 256);
				drn_check(CurrentQueryBatch.bOpen && CurrentQueryBatch.ElementCount == 0);
			}

			if (CurrentQueryBatch.StartElement > CurrentElement)
			{
				EndQueryBatchAndResolveQueryData(CmdList);
			}

			if (!CurrentQueryBatch.bOpen)
			{
				StartQueryBatch(CmdList, 256);
				drn_check(CurrentQueryBatch.bOpen && CurrentQueryBatch.ElementCount == 0);
			}
		}

		CurrentQueryBatch.ElementCount++;

		LastAllocatedElement = CurrentElement;
		drn_check(CurrentElement < QueryHeapCount);
		return CurrentElement;
	}

	void RenderQueryHeap::BeginQuery( D3D12CommandList* CmdList, RenderQuery* Query )
	{
		drn_check(CmdList);
		drn_check(CurrentQueryBatch.bOpen);

		Query->Reset();
		Query->HeapIndex = AllocQuery(CmdList);

		CmdList->GetD3D12CommandList()->BeginQuery( ActiveQueryHeap, QueryType, Query->HeapIndex );
	}

	void RenderQueryHeap::EndQuery( D3D12CommandList* CmdList, RenderQuery* Query )
	{
		drn_check(CmdList);

		if (QueryType == D3D12_QUERY_TYPE_OCCLUSION)
		{
			drn_check(CurrentQueryBatch.bOpen);
		}
		else
		{
			Query->FrameSubmitted = Renderer::Get()->GetFrameCount();
			Query->HeapIndex = AllocQuery(CmdList);
		}

		CmdList->GetD3D12CommandList()->EndQuery( ActiveQueryHeap, QueryType, Query->HeapIndex );

		CurrentQueryBatch.RenderQueries.push_back(Query);
	}

	uint32 RenderQueryHeap::GetNextElement( uint32 InElement )
	{
		InElement++;
		if (InElement >= QueryHeapCount)
		{
			InElement = 0;
		}

		return InElement;
	}

	uint32 RenderQueryHeap::GetNextBatchElement( uint32 InBatchElement )
	{
		InBatchElement++;

		if (InBatchElement >= (uint32) ActiveQueryBatches.size())
		{
			InBatchElement = 0;
		}

		return InBatchElement;
	}

	void RenderQueryHeap::CreateQueryHeap()
	{
		// Setup the query heap desc
		D3D12_QUERY_HEAP_DESC QueryHeapDesc;
		QueryHeapDesc.Type = (QueryType == D3D12_QUERY_TYPE_OCCLUSION)? D3D12_QUERY_HEAP_TYPE_OCCLUSION : D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		QueryHeapDesc.Count = QueryHeapCount;
		QueryHeapDesc.NodeMask = 0;

		GetParentDevice()->GetD3D12Device()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(ActiveQueryHeap.GetInitReference()));
		SetName(ActiveQueryHeap, "Query Heap");

		const D3D12_HEAP_PROPERTIES ResultBufferHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK, 0, 0);
		const D3D12_RESOURCE_DESC ResultBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ResultSize * QueryHeapDesc.Count);

		GetParentDevice()->CreateCommittedResource( ResultBufferDesc, ResultBufferHeapProperties, D3D12_RESOURCE_STATE_COPY_DEST, false, nullptr, ActiveResultBuffer.GetInitReference(), "Query Heap Result Buffer");
	}

	void RenderQueryHeap::DestroyQueryHeap()
	{
		ActiveQueryHeap = nullptr;
		ActiveResultBuffer = nullptr;
	}

	void RenderQuery::MarkResolved( D3D12CommandList* CommandList, RenderResource* InResultBuffer )
	{
		//CLSyncPoint = CommandList;
		SumbmittedFence = Renderer::Get()->GetFence()->Signal();

		ResultBuffer = InResultBuffer;
		bResolved = true;
	}

        }  // namespace Drn