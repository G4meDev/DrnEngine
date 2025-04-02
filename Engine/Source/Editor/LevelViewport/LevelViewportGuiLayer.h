#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class ViewportPanel;
	class WorldOutlinerPanel;
	class ActorDetailPanel;

	class LevelViewportGuiLayer : public ImGuiLayer
	{
	public:
		LevelViewportGuiLayer();
		virtual ~LevelViewportGuiLayer();

		virtual void Draw(float DeltaTime);

	protected:

		void DrawMenuBar(float DeltaTime);

		std::unique_ptr<ViewportPanel> m_ViewportPanel;
		std::unique_ptr<WorldOutlinerPanel> m_WorldOutlinerPanel;
		std::unique_ptr<ActorDetailPanel> m_ActorDetailPanel;

		bool m_ShowOutliner;
		bool m_ShowDetail;

	protected:
	};
}

#endif