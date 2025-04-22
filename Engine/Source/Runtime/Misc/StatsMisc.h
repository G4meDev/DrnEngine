#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct ScopeTimerData
	{
	public:
		ScopeTimerData()
			: Time(0)
		{
		}

		inline double GetTime() { return Time; }

	private:
		double Time;

		friend class ScopeTimer;
	};

	class ScopeTimer
	{
	public:

		ScopeTimer(ScopeTimerData& InData);
		~ScopeTimer();

		ScopeTimerData& Data;
		double m_StartTime;
	};
}