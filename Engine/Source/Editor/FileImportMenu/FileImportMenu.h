#pragma once

#if WITH_EDITOR

namespace Drn
{
	class FileImportMenuGuiLayer;

	class FileImportMenu
	{
	public:
		FileImportMenu(const std::string& InDisplayText, char* InFilters, std::function<void(std::string)> InOnSelectedFileDelegate);
		~FileImportMenu();

		static char* FileFilter_Any(); 
		static char* FileFilter_Model(); 
		static char* FileFilter_Texture(); 

	protected:
		std::unique_ptr<FileImportMenuGuiLayer> GuiLayer;

		const std::string m_DisplayText;
		const char* m_Filters;

		std::function<void(std::string)> m_OnSelectedFileDelegate;

		friend FileImportMenuGuiLayer;

	private:

	};
}

#endif