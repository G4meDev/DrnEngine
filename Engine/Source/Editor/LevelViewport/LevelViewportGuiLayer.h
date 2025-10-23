#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

#include "Runtime/Engine/PointLightActor.h"
#include "Runtime/Engine/SpotLightActor.h"
#include "Runtime/Engine/DirectionalLightActor.h"
#include "Runtime/Engine/SkyLightActor.h"
#include "Runtime/Engine/PostProcessVolume.h"

#include "Editor/LevelViewport/LevelViewport.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/EditorPanels/WorldOutlinerPanel.h"
#include "Editor/EditorPanels/ActorDetailPanel.h"
#include "Editor/EditorPanels/ModesPanel.h"

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
		void DrawBufferVisualizationMenu();

		void HandleViewportSpawnAsset(const std::string& AssetPath, const Vector& WorldPosition);
		void HandleViewportInputs();

		void HandleViewportSpawnEditorActor(class EditorLevelSpawnable* EditorActor, const Vector& WorldPosition);

		void OnHitPlay();
		void DrawContextPopup();

		void DeleteSelectedActor();
		void AlignSelectedComponentToSurfaceBelow();

		void OnScreenReprojectDropAsset(bool bHit, const Vector& WorldLocation, void* Payload);
		void OnScreenReprojectDropEditorActor(bool bHit, const Vector& WorldLocation, void* Payload);

		std::unique_ptr<ViewportPanel> m_ViewportPanel;
		std::unique_ptr<WorldOutlinerPanel> m_WorldOutlinerPanel;
		std::unique_ptr<ActorDetailPanel> m_ActorDetailPanel;
		std::unique_ptr<ModesPanel> m_ModesPanel;

		LevelViewport* m_OwningLevelViewport;

		bool m_ShowModes;
		bool m_ShowOutliner;
		bool m_ShowDetail;

		bool m_PlayingInEditor;

	protected:
	};

}

#endif