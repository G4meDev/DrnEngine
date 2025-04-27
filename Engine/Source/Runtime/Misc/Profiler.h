#pragma once

#include "ForwardTypes.h"
#include <fstream>

LOG_DECLARE_CATEGORY(LogProfiler);

namespace Drn
{
	enum class EProfileMode : uint8
	{
		Disabled,
		Capture_1,
		Capture_10,
		Capture_100
	};

	struct ProfileToken
	{
	public:
		ProfileToken(const std::string& InName, double InStartTime, double InDuration)
			: Name(InName)
			, StartTime(InStartTime)
			, Duration(InDuration)
		{
		}

		std::string Name;
		double StartTime;
		double Duration;
	};

	class Profiler
	{
	public:

		Profiler()
			: m_FrameIndex(0)
			, m_CaptureStartFrameIndex(0)
			, m_ProfileMode(EProfileMode::Disabled)
			, m_PendingProfileMode(EProfileMode::Disabled)
		{
		}

		~Profiler()
		{
		}

		static void Init();
		static void Shutdown();

		void Tick(float DeltaTime);

		inline static Profiler* Get() { return m_SingletonInstance; }
		inline bool IsProfiling() { return m_ProfileMode != EProfileMode::Disabled; }

		void Profile(EProfileMode Mode);
		void WriteToken(const ProfileToken& Token);

	protected:

	private:
		void StartProfiling();
		void EndProfiling();

		static Profiler* m_SingletonInstance;

		std::fstream m_File;
		EProfileMode m_ProfileMode;
		EProfileMode m_PendingProfileMode;
		uint64 m_FrameIndex;
		uint64 m_CaptureStartFrameIndex;
	};
}