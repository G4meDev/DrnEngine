#include "DrnPCH.h"
#include "Taskflow.h"

LOG_DEFINE_CATEGORY( LogTaskflow, "Taskflow" );

namespace Drn
{
	tf::Executor Taskflow::m_Executer = tf::Executor();

	void Taskflow::Init()
	{
		LOG(LogTaskflow, Info, "initializing taskflow");

	}

	void Taskflow::Shutdown()
	{
		LOG(LogTaskflow, Info, "shutting down taskflow");

	}


}