#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct ScopeTimerData
	{
	public:

		ScopeTimerData()
			: m_StartTime(0)
			, m_Duration(0)
		{
		}

		inline double GetStartTime() { return m_StartTime; }
		inline double GetDuration() { return m_Duration; }

	private:
		double m_StartTime;
		double m_Duration;

		friend class ScopeTimer;
	};

	class ScopeTimer
	{
	public:

		ScopeTimer(ScopeTimerData& InData);
		~ScopeTimer();

		ScopeTimerData& Data;
	};

	class ScopeProfilerStat
	{
	public:
		ScopeProfilerStat(std::string&& InName);
		ScopeProfilerStat(const std::string& InName, const char* Postfix);
		~ScopeProfilerStat();

	protected:

	private:
		std::string Name;
		double m_StartTime;
	};

#define PROFILER_ENABLED 1

#if PROFILER_ENABLED
	#define OPTICK_THREAD_TASK() OPTICK_THREAD(std::to_string(Taskflow::GetWorkerID()).c_str());
	#define SCOPE_STAT( ... ); OPTICK_EVENT( __VA_ARGS__ );
#else
	#define SCOPE_STAT( ... )
#endif
}