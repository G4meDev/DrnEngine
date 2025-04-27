#include "DrnPCH.h"
#include "Profiler.h"

LOG_DEFINE_CATEGORY( LogProfiler, "Profiler" );

namespace Drn
{
	Profiler* Profiler::m_SingletonInstance = nullptr;

	void Profiler::Init()
	{
		m_SingletonInstance = new Profiler();
	}

	void Profiler::Shutdown()
	{
		if (m_SingletonInstance)
		{
			delete m_SingletonInstance;
			m_SingletonInstance = nullptr;
		}
	}

	void Profiler::Tick( float DeltaTime )
	{
		SCOPE_STAT(ProfilerTick);

		m_FrameIndex++;

		if (IsProfiling())
		{
			// TODO: add appending mode to change at start of frame
			uint64 CaptureDuration = m_FrameIndex - m_CaptureStartFrameIndex;

			if ((m_ProfileMode == EProfileMode::Capture_1 && CaptureDuration > 1) ||
				(m_ProfileMode == EProfileMode::Capture_10 && CaptureDuration > 10) ||
				(m_ProfileMode == EProfileMode::Capture_100 && CaptureDuration > 100) )
			{
				EndProfiling();
			}
		}
	}

	void Profiler::StartProfiling( EProfileMode Mode )
	{
		if (!IsProfiling() && Mode != EProfileMode::Disabled)
		{
			LOG(LogProfiler, Warning, "profling started");
			m_ProfileMode = Mode;
			m_CaptureStartFrameIndex = m_FrameIndex;

			m_File = std::fstream( ".\\Saved\\Profiler\\profile.json", std::ios::out );
			if (!m_File)
			{
				LOG( LogProfiler, Error, "failed to create file. " );
				return;
			}

			m_File << "{\"otherData\": {},\"traceEvents\":[{}";
			m_File.flush();
		}
	}

	void Profiler::EndProfiling()
	{
		if (IsProfiling())
		{
			m_ProfileMode = EProfileMode::Disabled;

			m_File << "]}";
			m_File.close();

			LOG(LogProfiler, Warning, "profling finished.");
		}
	}

	void Profiler::WriteToken( const ProfileToken& Token )
	{
		if (IsProfiling())
		{
			std::stringstream json;

			json << std::setprecision( 3 ) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << Token.Duration << ',';
			json << "\"name\":\"" << Token.Name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << 0 << ",";
			json << "\"ts\":" << Token.StartTime;
			json << "}";

			m_File << json.str();
			m_File.flush();
		}
	}

}