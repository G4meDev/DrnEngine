#include "DrnPCH.h"
#include "FileImportMenu.h"

#if WITH_EDITOR

#include "FileImportMenuGuiLayer.h"

namespace Drn
{
	FileImportMenu::FileImportMenu()
	{
		GuiLayer = std::make_unique<FileImportMenuGuiLayer>(this);
		GuiLayer->Attach();
	}

	FileImportMenu::~FileImportMenu()
	{
		GuiLayer->DeAttach();
		GuiLayer.reset();
	}
}

#endif