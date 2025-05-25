#pragma once

#if WITH_EDITOR

namespace Drn
{
	class FileImportMenuGuiLayer;

	class FileImportMenu
	{
	public:
		FileImportMenu(const std::string& InDisplayText, const char* InFilters, std::function<void(std::string)> InOnSelectedFileDelegate);
		~FileImportMenu();

		const static char* FileFilter_Any(); 
		const static char* FileFilter_Model(); 
		const static char* FileFilter_Texture(); 

	protected:
		FileImportMenuGuiLayer* GuiLayer;

		const std::string m_DisplayText;
		const char* m_Filters;

		std::function<void(std::string)> m_OnSelectedFileDelegate;

		friend FileImportMenuGuiLayer;

	private:

	};
}

#endif