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

		static FORCEINLINE float GetApplicationDeltaTime() { return m_ApplicationDeltaTime; }

	private:

		static FORCEINLINE void SetApplicationDeltaTime( float DeltaTime ) { m_ApplicationDeltaTime = DeltaTime; }

		static float m_ApplicationDeltaTime;

		friend class Application;
	};
}