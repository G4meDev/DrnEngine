#include "DrnPCH.h"
#include "Time.h"

namespace Drn
{
	double Time::SecondsPerCycle = 0;

	void Time::Init()
	{
		LARGE_INTEGER fr;
		QueryPerformanceFrequency( &fr );
		Time::SecondsPerCycle = fr.QuadPart;
	}


}