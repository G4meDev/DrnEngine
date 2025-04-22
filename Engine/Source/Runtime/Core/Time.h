#pragma once

#include <windows.h>

namespace Drn
{
	class Time
	{
	public:

		static double SecondsPerCycle;

		static void Init();

		static FORCEINLINE double GetSeconds()
		{
			LARGE_INTEGER Cycles;
			QueryPerformanceCounter(&Cycles);

			return Cycles.QuadPart / SecondsPerCycle;
		}
	};
}