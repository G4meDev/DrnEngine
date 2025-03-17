#include "DrnPCH.h"
#include "Log.h"

#include "Editor/OutputLog/OutputLog.h"

#ifndef DRN_DIST

Verbosity Log::VerboseLevel = Verbosity::Info;

LogCategory::LogCategory(char* InName)
{
	Name = InName;

#if WITH_EDITOR
	Drn::OutputLog::Get()->LogCategories.push_back(this);
#endif
}

std::unordered_map<Verbosity, int> Log::ColorCodes =
{
	{Verbosity::Info, 15},
	{Verbosity::Warning, 9},
	{Verbosity::Error, 4}
};

void PrintLogToConsole(const LogMessage& InLogMessage)
{
	// find color associated with verbose level
	std::unordered_map<const Verbosity, int>::const_iterator f =
		Log::ColorCodes.find(InLogMessage.VerboseLevel);

	// change console output color
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, f->second);

	std::cout << InLogMessage.Time.ToString();
	std::cout << InLogMessage.Category->Name;
	std::cout << InLogMessage.Message;

	// set console color to default
	SetConsoleTextAttribute(h, 15);

#if WITH_EDITOR
	Drn::OutputLog::Get()->AddLogMessage(InLogMessage);
#endif
}

#endif

