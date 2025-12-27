#include "DrnPCH.h"
#include "GpuProfiler.h"

#define MAX_QUERIES UINT16_MAX

namespace Drn
{
	GpuProfiler* GpuProfiler::Instance = nullptr;

	GpuProfiler::GpuProfiler()
	{
		QueryPool = new RenderQueryPool(Renderer::Get()->GetDevice(), ERenderQueryType::AbsoluteTime, MAX_QUERIES);

		for (int32 i = 0; i < NUM_BACKBUFFERS; i++)
		{
			PendingFrames[i] = new GpuProfilerFrame();
		}
	}

	GpuProfilerFrame* GpuProfiler::GetCurrentFrame()
	{
		return PendingFrames[CurrentFrameIndex % NUM_BACKBUFFERS];
	}

	GpuProfiler* GpuProfiler::Get()
	{
		if (Instance == nullptr)
		{
			Instance = new GpuProfiler();
		}

		return Instance;
	}

	void GpuProfiler::SafeRelease()
	{
		if (Instance)
		{
			Instance->Cleanup();
			delete Instance;
			Instance = nullptr;
		}
	}

	void GpuProfiler::PushStat( D3D12CommandList* CmdList, const std::string& Name )
	{
		if (Renderer::Get()->GetFrameCount() > CurrentFrameIndex)
		{
			CurrentFrameIndex = Renderer::Get()->GetFrameCount();
			TryResolve();
		}

		PooledRenderQuery A = QueryPool->AllocateQuery();
		PooledRenderQuery B = QueryPool->AllocateQuery();

		GetCurrentFrame()->ScopeStack.push_back({Name, QueryPool->AllocateQuery(), QueryPool->AllocateQuery()});
		CmdList->EndRenderQuery(GetCurrentFrame()->ScopeStack.back().StartQuery.GetQuery());
	}

	void GpuProfiler::PopStat( D3D12CommandList* CmdList )
	{
		drn_check(GetCurrentFrame());
		drn_check(!GetCurrentFrame()->ScopeStack.empty());

		CmdList->EndRenderQuery(GetCurrentFrame()->ScopeStack.back().EndQuery.GetQuery());
		GetCurrentFrame()->PendingToResolve.push_back(std::move(GetCurrentFrame()->ScopeStack.back()));
		GetCurrentFrame()->ScopeStack.pop_back();
	}

	void GpuProfiler::TryResolve()
	{
		Timings.clear();

		for ( GpuProfileScope& Scope : GetCurrentFrame()->PendingToResolve )
		{
			void* SPtr;
			Scope.StartQuery.GetQuery()->ResultBuffer->GetResource()->Map(0, nullptr, &SPtr);
			void* EPtr;
			Scope.EndQuery.GetQuery()->ResultBuffer->GetResource()->Map(0, nullptr, &EPtr);
			uint64 StartTime = ((uint64*)SPtr)[Scope.StartQuery.GetQuery()->HeapIndex];
			uint64 EndTime = ((uint64*)EPtr)[Scope.EndQuery.GetQuery()->HeapIndex];

			uint64 Freq;
			Renderer::Get()->GetCommandQueue()->GetTimestampFrequency(&Freq);
			double Duration = (double)(EndTime - StartTime) / Freq;

			auto it = Timings.find(Scope.Name);
			if (it != Timings.end())
			{
				Duration += it->second; 
			}

			Timings[Scope.Name] = Duration;
		}

		drn_check(GetCurrentFrame()->ScopeStack.empty());
		GetCurrentFrame()->PendingToResolve.clear();
	}

	void GpuProfiler::Cleanup()
	{
		for (int32 i = 0; i < NUM_BACKBUFFERS; i++)
		{
			delete PendingFrames[i];
		}

		QueryPool.SafeRelease();
	}

}  // namespace Drn