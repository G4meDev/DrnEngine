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
		ScopeProfilerStat(const std::string& InName);
		ScopeProfilerStat(const std::string& InName, const char* Postfix);
		~ScopeProfilerStat();

	protected:

	private:
		std::string Name;
		double m_StartTime;
	};

#define PROFILER_ENABLED 1

#if PROFILER_ENABLED
	#define SCOPE_STAT( name ) ScopeProfilerStat Scope_Stat_##name (#name)
	#define SCOPE_STAT_POSTFIX( name , postfix ) ScopeProfilerStat Scope_Stat_##name (#name , postfix)
#else
	#define SCOPE_STAT( name )
	#define SCOPE_STAT_POSTFIX( name , postfix )
#endif
}