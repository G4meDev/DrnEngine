#include "DrnPCH.h"
#include "DateTime.h"

namespace Drn
{
	DateTime DateTime::Now()
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		time_t Ttime = std::chrono::system_clock::to_time_t(now);
		tm* LocalTime = localtime(&Ttime);

		DateTime Result;
		Result.Year = LocalTime->tm_year + 1900;
		Result.Month = LocalTime->tm_mon + 1;
		Result.Day = LocalTime->tm_mday;
		Result.Hour = LocalTime->tm_hour;
		Result.Minute = LocalTime->tm_min;
		Result.Second = LocalTime->tm_sec;

		return Result;
	}

	std::string DateTime::ToString() const
	{
		std::stringstream ss;

		// [%d/%m/%Y_%H:%M:%S]
		ss << "[" << Day << "/" << Month << "/" << Year << "_" << Hour << ":" << Minute << ":" << Second << "]";

		return ss.str();
	}
}