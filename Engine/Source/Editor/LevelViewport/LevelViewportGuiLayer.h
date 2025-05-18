#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

struct ImGuiPayload;

namespace Drn
{
	class LevelViewport;
	class ViewportPanel;
	class WorldOutlinerPanel;
	class ActorDetailPanel;

	class LevelViewportGuiLayer : public ImGuiLayer
	{
	public:
		LevelViewportGuiLayer( LevelViewport* InOwningLevelViewport );
		virtual ~LevelViewportGuiLayer();

		virtual void Draw(float DeltaTime);

	protected:

		void DrawMenuBar(float DeltaTime);
		void DrawViewportMenu(float DeltaTime);

		void HandleViewportPayload(const ImGuiPayload* Payload);

		void OnHitPlay();
		void ShowContextPopup();
		void DrawContextPopup();

		std::unique_ptr<ViewportPanel> m_ViewportPanel;
		std::unique_ptr<WorldOutlinerPanel> m_WorldOutlinerPanel;
		std::unique_ptr<ActorDetailPanel> m_ActorDetailPanel;

		LevelViewport* m_OwningLevelViewport;

		bool m_ShowOutliner;
		bool m_ShowDetail;

		bool m_PlayingInEditor;

	protected:
	};
}

#endif