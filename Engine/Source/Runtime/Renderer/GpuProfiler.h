#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class D3D12CommandList;
	class RenderQueryPool;

	struct GpuProfileScope
	{
		std::string Name;

		PooledRenderQuery StartQuery;
		PooledRenderQuery EndQuery;
	};

	struct GpuProfilerFrame
	{
		std::vector<GpuProfileScope> ScopeStack;
		std::vector<GpuProfileScope> PendingToResolve;
	};

	class GpuProfiler
	{
	public:
		static GpuProfiler* Get();
		static void SafeRelease();

		const std::unordered_map<std::string, double>& GetTimings() { return Timings; }

		void PushStat(D3D12CommandList* CmdList, const std::string& Name);
		void PopStat(D3D12CommandList* CmdList);

		void TryResolve();

	private:
		static GpuProfiler* Instance;
		GpuProfiler();

		GpuProfilerFrame* GetCurrentFrame();

		void Cleanup();
		std::unordered_map<std::string, double> Timings;

		GpuProfilerFrame* PendingFrames[NUM_BACKBUFFERS];
		TRefCountPtr<RenderQueryPool> QueryPool;

		uint64 CurrentFrameIndex = 0;
	};

	class GpuProfilerScopedStat
	{
		D3D12CommandList* CmdList;

	public:
		GpuProfilerScopedStat(D3D12CommandList* InCmdList, const std::string& Name)
			: CmdList(InCmdList)
		{
			GpuProfiler::Get()->PushStat(InCmdList, Name);
		}

		~GpuProfilerScopedStat()
		{
			GpuProfiler::Get()->PopStat(CmdList);
		};
	};

#define CAT(a, b) a ## b
#define SCOPED_GPU_STAT( CmdList , Name ) GpuProfilerScopedStat CAT(GPUStatEvent_##StatName, __LINE__)( CmdList , Name );
}