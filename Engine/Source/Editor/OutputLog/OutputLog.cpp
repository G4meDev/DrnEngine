#include "DrnPCH.h"
#include "OutputLog.h"

#if WITH_EDITOR
#include "Editor/OutputLog/OutputLogGuiLayer.h"

namespace Drn
{
	std::unique_ptr<OutputLog> OutputLog::SingletonInstance;

	OutputLog::OutputLog()
	{
		// TODO: when container is full remove half of it and shift other half to first
		LogMessages.reserve(2000);
	}

	OutputLog::~OutputLog()
	{ }

	void OutputLog::Init()
	{
		OutputLogLayer = std::make_unique<OutputLogGuiLayer>();
		OutputLogLayer->Attach();

	}

	void OutputLog::Tick(float DeltaTime)
	{
		
	}

	OutputLog* OutputLog::Get()
	{
		if (!SingletonInstance.get())
		{
			SingletonInstance = std::make_unique<OutputLog>();
		}

		return SingletonInstance.get();
	}

	const std::vector<LogMessage>& OutputLog::GetLogMessages() const
	{
		return LogMessages;
	}

	void OutputLog::AddLogMessage(const LogMessage& InLogMessage)
	{
		LogMessages.push_back(InLogMessage);

	}

	void OutputLog::AddLogMessage(LogCategory* InLogCategory, Verbosity InVerbosity, const DateTime& InTime, const std::string& InMessage)
	{
		AddLogMessage(LogMessage(InLogCategory, InVerbosity, InTime, InMessage));
	}

}

#endif