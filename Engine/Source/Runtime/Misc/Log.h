#pragma once

#include <unordered_map>
#include "Runtime/Misc/DateTime.h"

#ifndef DRN_DIST

#define LOG_MAX_CHARACTERS 500

//#define LOG_DECLARE_CATEGORY(Category)													\
//extern LogCategory Category;															\


#define LOG_DEFINE_CATEGORY(Category , Name)											\
LogCategory Category(Name);

#define LOG(Category , Verbose , Format , ...)											\
{																						\
	Verbosity VerbosityLevel = Verbosity::##Verbose;									\
	Drn::DateTime DTime = Drn::DateTime::Now();										\
	char MsgText[LOG_MAX_CHARACTERS];													\
	std::snprintf(MsgText, LOG_MAX_CHARACTERS, Format##"\n", __VA_ARGS__);				\
	LogMessage LogMsg(&Category, VerbosityLevel, DTime, MsgText);						\
	PrintLogToConsole(LogMsg);															\
}


#define LOG_VERBOSE_LEVEL(Verbose)														\
Log::VerboseLevel = Verbosity::##Verbose;

#define LOG_ENABLE_CATEGORY(Category)													\
Category.Suppressed = false;

#define LOG_DISABLE_CATEGORY(Category)													\
Category.Suppressed = true;

#else
#define LOG_DECLARE_CATEGORY(Category)	
#define LOG_DEFINE_CATEGORY(Category , Name)
#define LOG(Category , Verbose , Format , ...)
#define LOG_VERBOSE_LEVEL(Verbose)
#define LOG_ENABLE_CATEGORY(Category)
#define LOG_DISABLE_CATEGORY(Category)
#endif 


enum class Verbosity : unsigned short int
{
	Error = 0,
	Warning,
	Info
};

class Log
{
public:
	static Verbosity VerboseLevel;
	static std::unordered_map<Verbosity, int> ColorCodes;
};

struct LogCategory
{
	LogCategory(char* InName);

	char* Name;
	bool Suppressed = false;
};

struct LogMessage
{
	LogMessage(LogCategory* InCategory, Verbosity InVerboseLevel, const Drn::DateTime& InTime, const std::string& InMessage)
		: Category(InCategory), VerboseLevel(InVerboseLevel), Time(InTime), Message(InMessage)
	{ }

	LogCategory* Category;
	Verbosity VerboseLevel;

	Drn::DateTime Time;
	std::string Message;
};

void PrintLogToConsole(const LogMessage& InLogMessage);