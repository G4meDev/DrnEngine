#pragma once

#include "taskflow.hpp"

LOG_DECLARE_CATEGORY(LogTaskflow);

namespace Drn
{
	class Taskflow
	{
	public:
		static void Init();
		static void Shutdown();

		inline static tf::Executor& GetExecuter() { return m_Executer; };
		inline static int GetWorkerID() { return m_Executer.this_worker_id(); }

	private:
		static tf::Executor m_Executer;
	};
}