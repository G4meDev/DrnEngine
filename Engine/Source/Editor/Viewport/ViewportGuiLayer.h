#pragma once

#if WITH_EDITOR
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class ViewportGuiLayer : public ImGuiLayer
	{
	public:
		virtual void Draw() override;


	private:
		//void ShowDisplayBufferMenuItem();
		//void ShowResolutionOption();

		void OnViewportSizeChanged(const IntPoint& OldSize, const IntPoint& NewSize);

		IntPoint ViewportImageSize = IntPoint(800, 600);

		friend class Viewport;
	};

}

#endif