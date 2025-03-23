#pragma once

#if WITH_EDITOR
#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

LOG_DECLARE_CATEGORY( LogContentBrowser );

namespace Drn
{
	struct SystemFileNode;

	class ContentBrowserGuiLayer: public ImGuiLayer
	{
	public:
		ContentBrowserGuiLayer();
		~ContentBrowserGuiLayer();

		virtual void Draw() override;

	private:

		void DrawNextFolder(SystemFileNode* Node);


		void OnImport();
		void OnRefresh();

		std::unique_ptr<SystemFileNode> RootFolder;
		friend class ContentBrowser;
	};
}

#endif