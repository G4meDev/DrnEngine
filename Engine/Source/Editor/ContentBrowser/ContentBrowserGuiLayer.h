#pragma once

#if WITH_EDITOR
#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

LOG_DECLARE_CATEGORY( LogContentBrowser );

namespace Drn
{
	class ContentBrowserGuiLayer: public ImGuiLayer
	{
	public:
		ContentBrowserGuiLayer();
		~ContentBrowserGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	private:

		void DrawNextFolder(SystemFileNode* Node);
		void DrawFileView();

		void OnImport();
		void OnRefresh();

		void OnSelectedFileToImport(std::string FilePath);

		void ConvertPath(SystemFileNode* Node);

		void GetDirectoryWithPath(SystemFileNode* Node, const std::string& Path, SystemFileNode** Result);

		std::unique_ptr<SystemFileNode> EngineRootFolder;
		std::unique_ptr<SystemFileNode> GameRootFolder;

		SystemFileNode* SelectedFolder;
		std::vector<SystemFileNode*> SelectedFolderFiles;
		SystemFileNode* SelectedFile;

		std::shared_ptr<Window> ImportWindow;

		friend class ContentBrowser;
	};
}

#endif