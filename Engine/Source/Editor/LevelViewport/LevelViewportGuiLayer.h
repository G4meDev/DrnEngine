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
		void HandleViewportInputs();

		void OnHitPlay();
		void DrawContextPopup();

		void DeleteSelectedActor();
		void AlignSelectedComponentToSurfaceBelow();

		template<typename T>
		T* SpawnActorFromClassInLevel(const std::string SpawnName);

		std::unique_ptr<ViewportPanel> m_ViewportPanel;
		std::unique_ptr<WorldOutlinerPanel> m_WorldOutlinerPanel;
		std::unique_ptr<ActorDetailPanel> m_ActorDetailPanel;

		LevelViewport* m_OwningLevelViewport;

		bool m_ShowOutliner;
		bool m_ShowDetail;

		bool m_PlayingInEditor;

	protected:
	};

	template<typename T>
	T* LevelViewportGuiLayer::SpawnActorFromClassInLevel( const std::string SpawnName )
	{
		T* SpawnedActor = m_OwningLevelViewport->m_OwningWorld->SpawnActor<T>();
		SpawnedActor->SetActorLabel(SpawnName);
		return SpawnedActor;
	}

}

#endif