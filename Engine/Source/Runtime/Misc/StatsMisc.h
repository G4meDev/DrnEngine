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
}