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

		friend class ContentBrowser;

		void OnImport();
		void OnRefresh();
	};
}

#endif