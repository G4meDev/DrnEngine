#include "DrnPCH.h"
#include "FileImportMenuGuiLayer.h"

#if WITH_EDITOR

#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "FileImportMenu.h"

#include "Runtime/Misc/Path.h"
#include "Editor/Editor.h"

LOG_DEFINE_CATEGORY( LogFileImportMenu, "FileImportMenu" );

namespace Drn
{
	FileImportMenuGuiLayer::FileImportMenuGuiLayer(FileImportMenu* InFileImportMenu)
		: m_FileImportMenu(InFileImportMenu)
	{
		IGFD::FileDialogConfig config;
		config.path = Path::GetContentPath();
		ImGuiFileDialog::Instance()->OpenDialog( "ChooseFileDlgKey", m_FileImportMenu->m_DisplayText, m_FileImportMenu->m_Filters, config );

		LOG( LogFileImportMenu, Info, "opened file import menu." );
	}

	FileImportMenuGuiLayer::~FileImportMenuGuiLayer()
	{
		
	}

	void FileImportMenuGuiLayer::Draw()
	{
		if ( ImGuiFileDialog::Instance()->Display( "ChooseFileDlgKey" ) )
		{
			if ( ImGuiFileDialog::Instance()->IsOk() )
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

				if (m_FileImportMenu && m_FileImportMenu->m_OnSelectedFileDelegate)
				{
					m_FileImportMenu->m_OnSelectedFileDelegate(filePathName);
				}
			}
		
			ImGuiFileDialog::Instance()->Close();

			LOG( LogFileImportMenu, Info, "closed file import menu." );
			Editor::Get()->CloseImportMenu();

			m_Open = false;
		}
	}
}

#endif