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
}