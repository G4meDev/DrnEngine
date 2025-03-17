#pragma once

#if WITH_EDITOR

namespace Drn
{
	class OutputLogGuiLayer;

	class OutputLog
	{
	public:
		OutputLog();
		~OutputLog();

		void Init();
		void Tick(float DeltaTime);

		static OutputLog* Get();

		const std::vector<LogMessage>& GetLogMessages() const;
		void AddLogMessage(const LogMessage& InLogMessage);
		void AddLogMessage(LogCategory* InLogCategory, Verbosity InVerbosity,
			const DateTime& InTime, const std::string& InMessage);

		std::vector<LogCategory*> LogCategories;


	protected:
		std::unique_ptr<OutputLogGuiLayer> OutputLogLayer;
		std::vector<LogMessage> LogMessages;

		bool bInfoLogEnabled = true;
		bool bWarningLogEnabled = true;
		bool bErrorLogEnabled = true;

		bool bShowTime = true;
		bool bShowCategory = true;

	private:
		static std::unique_ptr<OutputLog> SingletonInstance;

		friend class OutputLogGuiLayer;
	};
}

#endif