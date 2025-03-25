#include "DrnPCH.h"
#include "FileImportMenuGuiLayer.h"

#if WITH_EDITOR

#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "FileImportMenu.h"

#include "Editor/Editor.h"

LOG_DEFINE_CATEGORY( LogFileImportMenu, "FileImportMenu" );

namespace Drn
{
	FileImportMenuGuiLayer::FileImportMenuGuiLayer(FileImportMenu* InFileImportMenu)
		: m_FileImportMenu(InFileImportMenu)
	{
		IGFD::FileDialogConfig config;
		config.path = ".";
		ImGuiFileDialog::Instance()->OpenDialog( "ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", config );
	}

	FileImportMenuGuiLayer::~FileImportMenuGuiLayer()
	{
		//IGFD::FileDialogConfig config;
		//config.path = ".";
		//ImGuiFileDialog::Instance()->( "ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", config );
	}

	void FileImportMenuGuiLayer::Draw()
	{
		if ( ImGuiFileDialog::Instance()->Display( "ChooseFileDlgKey" ) )
		{
			if ( ImGuiFileDialog::Instance()->IsOk() )
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				std::string filePath     = ImGuiFileDialog::Instance()->GetCurrentPath();
		
				LOG( LogFileImportMenu, Info, "%s", filePathName.c_str());
				LOG( LogFileImportMenu, Info, "%s", filePath.c_str());
			}
		
			else
			{
				ImGuiFileDialog::Instance()->Close();

				LOG( LogFileImportMenu, Info, "closed file import menu." );

				Editor::Get()->CloseImportMenu();
			}
		}
	}
}

#endif