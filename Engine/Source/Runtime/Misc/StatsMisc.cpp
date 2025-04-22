#include "DrnPCH.h"
#include "StatsMisc.h"

namespace Drn
{
	ScopeTimer::ScopeTimer( ScopeTimerData& InData )
		: m_StartTime(Time::GetSeconds())
		, Data(InData)
	{
	}

	ScopeTimer::~ScopeTimer()
	{
		Data.Time = Time::GetSeconds() - m_StartTime;
	}

}