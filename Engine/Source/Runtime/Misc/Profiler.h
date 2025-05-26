#pragma once

#include "ForwardTypes.h"
#include <fstream>

LOG_DECLARE_CATEGORY(LogProfiler);

#define PROFILER_TOKEN_BUFFER_SIZE 1024

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
		ProfileToken(std::string&& InName, double InStartTime, double InDuration)
			: Name(std::move(InName))
			, StartTime(InStartTime)
			, Duration(InDuration)
		{
		}

		ProfileToken()
			: ProfileToken("NameNull", 0, 0)
		{
		}

		ProfileToken& operator=(ProfileToken&& Other) noexcept
		{
			Name = std::move(Other.Name);
			StartTime = Other.StartTime;
			Duration = Other.Duration;

			return *this;
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
			, m_TokenBufferCount(0)
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
		void WriteToken(ProfileToken&& Token);

	protected:

	private:
		void StartProfiling();
		void EndProfiling();

		void FlushBuffer();

		inline bool IsBufferFull() const { return m_TokenBufferCount >= PROFILER_TOKEN_BUFFER_SIZE; }

		static Profiler* m_SingletonInstance;

		std::fstream m_File;
		EProfileMode m_ProfileMode;
		EProfileMode m_PendingProfileMode;
		uint64 m_FrameIndex;
		uint64 m_CaptureStartFrameIndex;

		ProfileToken m_TokenBuffer[PROFILER_TOKEN_BUFFER_SIZE];
		uint32 m_TokenBufferCount;
	};
}