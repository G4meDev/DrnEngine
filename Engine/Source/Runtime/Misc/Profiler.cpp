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

	void Profiler::StartProfiling()
	{
		if (!m_Profiling)
		{
			m_Profiling = true;
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
		if (m_Profiling)
		{
			m_Profiling = false;

			m_File << "]}";
			m_File.close();
		}
	}

	void Profiler::WriteToken( const ProfileToken& Token )
	{
		if (m_Profiling)
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