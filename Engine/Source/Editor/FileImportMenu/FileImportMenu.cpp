#include "DrnPCH.h"
#include "FileImportMenu.h"

#if WITH_EDITOR

#include "FileImportMenuGuiLayer.h"

namespace Drn
{
	FileImportMenu::FileImportMenu(const std::string& InDisplayText, char* InFilters, std::function<void(std::string)> InOnSelectedFileDelegate)
		: m_DisplayText(InDisplayText)
		, m_Filters(InFilters)
		, m_OnSelectedFileDelegate(InOnSelectedFileDelegate)
	{
		GuiLayer = std::make_unique<FileImportMenuGuiLayer>(this);
		GuiLayer->Attach();
	}

	FileImportMenu::~FileImportMenu()
	{
		GuiLayer->DeAttach();
		GuiLayer.reset();
	}

	char* FileImportMenu::FileFilter_Any()
	{
		// TODO: better filter group
		// check https://github.com/aiekick/ImGuiFileDialog/blob/master/Documentation.md#filter-collections-

		return ".*";
	}

	char* FileImportMenu::FileFilter_Model()
	{
		return ".obj,.fbx";
	}

	char* FileImportMenu::FileFilter_Texture()
	{
		return ".png,.jpg,.tga";
	}
}

#endif