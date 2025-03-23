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

		virtual void Draw() override;

	private:

		void DrawNextFolder(SystemFileNode* Node);
		void DrawFileView();

		void OnImport();
		void OnRefresh();

		std::unique_ptr<SystemFileNode> RootFolder;
		SystemFileNode* SelectedFolder;
		std::vector<SystemFileNode*> SelectedFolderFiles;
		SystemFileNode* SelectedFile;

		float LayoutOuterPadding = 32;

		friend class ContentBrowser;
	};
}

#endif