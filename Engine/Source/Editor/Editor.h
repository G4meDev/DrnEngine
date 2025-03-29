#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogEditor);

namespace Drn
{
	class FileImportMenu;

	class Editor
	{
	public:
		Editor();
		~Editor();

		void Init();
		void Tick(float DeltaTime);
		void Shutdown();

		static Editor* Get();

		std::shared_ptr<FileImportMenu> OpenImportMenu(const std::string& InDisplayText, char* InFilters, std::function<void(std::string)> InOnSelectedFile);
		void CloseImportMenu();

	protected:

		std::shared_ptr<FileImportMenu> m_FileImportMenu;

	private:

		void OnSelectedFile(const std::string Path);

		static std::unique_ptr<Editor> SingletonInstance;

		friend class ContentBrowserGuiLayer;
	};

}

#endif