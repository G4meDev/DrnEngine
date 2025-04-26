#pragma once

#include "ForwardTypes.h"
#include <fstream>

LOG_DECLARE_CATEGORY(LogProfiler);

namespace Drn
{
	struct ProfileToken
	{
	public:
		ProfileToken(const std::string& InName, float InStartTime, float InDuration)
			: Name(InName)
			, StartTime(InStartTime)
			, Duration(InDuration)
		{
		}

		std::string Name;
		float StartTime;
		float Duration;
	};

	class Profiler
	{
	public:

		Profiler()
			: m_Profiling(false)
		{
		}

		~Profiler()
		{
		}

		static void Init();
		static void Shutdown();

		inline static Profiler* Get() { return m_SingletonInstance; }
		inline bool IsProfiling() { return m_Profiling; }

		void StartProfiling();
		void EndProfiling();

		void WriteToken(const ProfileToken& Token);

	protected:

	private:
		static Profiler* m_SingletonInstance;

		bool m_Profiling;
		std::fstream m_File;
	};
}
