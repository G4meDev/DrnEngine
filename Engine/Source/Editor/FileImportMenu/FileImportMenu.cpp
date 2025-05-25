#include "DrnPCH.h"
#include "FileImportMenu.h"

#if WITH_EDITOR

#include "FileImportMenuGuiLayer.h"

namespace Drn
{
	FileImportMenu::FileImportMenu(const std::string& InDisplayText, const char* InFilters, std::function<void(std::string)> InOnSelectedFileDelegate)
		: m_DisplayText(InDisplayText)
		, m_Filters(InFilters)
		, m_OnSelectedFileDelegate(InOnSelectedFileDelegate)
	{
		GuiLayer = new FileImportMenuGuiLayer(this);
		GuiLayer->Attach();
	}

	FileImportMenu::~FileImportMenu()
	{
		m_OnSelectedFileDelegate = nullptr;

		//GuiLayer->DeAttach();
		//GuiLayer.reset();
	}

	const char* FileImportMenu::FileFilter_Any()
	{
		// TODO: better filter group
		// check https://github.com/aiekick/ImGuiFileDialog/blob/master/Documentation.md#filter-collections-

		return ".*";
	}

	const char* FileImportMenu::FileFilter_Model()
	{
		return ".obj,.fbx";
	}

	const char* FileImportMenu::FileFilter_Texture()
	{
		return ".png,.jpg,.tga";
	}
}

#endif