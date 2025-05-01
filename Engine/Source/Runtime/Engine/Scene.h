#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogScene);

namespace Drn
{
	class PrimitiveSceneProxy;

	class Scene
	{
	public:
		Scene(World* InWorld);
		~Scene();

		inline void Release() { delete this; }

		inline World* GetWorld() { return m_World; }

		void Render(dx12lib::CommandList* CommandList);

		SceneRenderer* AllocateSceneRenderer();
		void ReleaseSceneRenderer(SceneRenderer*& InSceneRenderer);

		void InitSceneRender(dx12lib::CommandList* CommandList);

		void AddPrimitiveProxy(PrimitiveSceneProxy* InPrimitiveSceneProxy);
		void RemovePrimitiveProxy(PrimitiveSceneProxy* InPrimitiveSceneProxy);


	protected:

		void OnNewActors(const std::set<Actor*>& NewActors);
		void OnRemoveActor(const Actor* RemovedActor);

		World* m_World;

		std::set<PrimitiveSceneProxy*> m_PrimitiveProxies;
		std::set<PrimitiveSceneProxy*> m_PendingProxies;

		std::set<SceneRenderer*> m_SceneRenderers;

		friend class SceneRenderer;

	private:

	};
}