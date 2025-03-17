#pragma once

#include <string>
#include "Runtime/Misc/DateTime.h"

namespace Drn
{
	enum class DayOfWeek
	{
		Monday = 0,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday,
		Sunday
	};

	enum class MonthOfYear
	{
		January = 1,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};

	struct DateTime
	{
		DateTime(int32 InYear, int32 InMonth, int32 InDay, int32 InHour, int32 InMinuts, int32 inSeconds)
			: Year(InYear), Month(InMonth), Day(InDay), Hour(InHour), Minute(InMinuts), Second(inSeconds)
		{ }

		DateTime() : DateTime(0, 0, 0, 0, 0, 0)
		{ }

		static DateTime Now();

		std::string ToString() const;

		int32 Year;
		int32 Month;
		int32 Day;

		int32 Hour;
		int32 Minute;
		int32 Second;
	};
}