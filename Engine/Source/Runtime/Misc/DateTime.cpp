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
		SCOPE_STAT();

		std::string Str(21, 0);
		sprintf(Str.data(), "[%02d/%02d/%04d-%02d:%02d:%02d]", Day, Month, Year, Hour, Minute, Second);
		
		return Str;

		//return std::format("[{:02}/{:02}/{:04}-{:02}:{:02}:{:02}]", Day, Month, Year, Hour, Minute, Second);
	}

	std::string DateTime::ToStringFileStamp() const
	{
		SCOPE_STAT();

		std::string Str(19, 0);
		sprintf(Str.data(), "%04d.%02d.%02d-%02d.%02d.%02d", Year, Month, Day, Hour, Minute, Second);
		
		return Str;
	}

}