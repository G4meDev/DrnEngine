#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class ViewportPanel;
	class WorldOutlinerPanel;;

	class LevelViewportGuiLayer : public ImGuiLayer
	{
	public:
		LevelViewportGuiLayer();
		virtual ~LevelViewportGuiLayer();

		virtual void Draw(float DeltaTime);

	protected:

		std::unique_ptr<ViewportPanel> m_ViewportPanel;
		std::unique_ptr<WorldOutlinerPanel> m_WorldOutlinerPanel;

	protected:
	};
}

#endif