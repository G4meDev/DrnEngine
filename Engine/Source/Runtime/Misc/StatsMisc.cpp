#include "DrnPCH.h"
#include "StatsMisc.h"

namespace Drn
{
	ScopeTimer::ScopeTimer( ScopeTimerData& InData )
		: Data(InData)
	{
		InData.m_StartTime = Time::GetSeconds();
	}

	ScopeTimer::~ScopeTimer()
	{
		Data.m_Duration = Time::GetSeconds() - Data.m_StartTime;
	}

	ScopeProfilerStat::ScopeProfilerStat( const std::string& InName ) 
	{
		Name = InName;
		m_StartTime = Time::GetSeconds();
	}

	ScopeProfilerStat::ScopeProfilerStat( const std::string& InName, const char* Postfix )
	{
		Name = InName + Postfix;
		m_StartTime = Time::GetSeconds();
	}

	ScopeProfilerStat::~ScopeProfilerStat()
	{
		Profiler::Get()->WriteToken(ProfileToken(Name, m_StartTime * 1000000.0, (Time::GetSeconds() - m_StartTime) * 1000000.0));
	}
}