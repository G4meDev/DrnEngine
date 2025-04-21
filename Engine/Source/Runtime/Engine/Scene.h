#pragma once

#include "ForwardTypes.h"

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

		void AddStaticMeshCompponent(StaticMeshComponent* InStaticMesh);
		void RemoveStaticMeshCompponent(StaticMeshComponent* InStaticMesh);

		void InitSceneRender(dx12lib::CommandList* CommandList);

		void AddPrimitiveProxy(PrimitiveSceneProxy* InPrimitiveSceneProxy);
		void RemovePrimitiveProxy(PrimitiveSceneProxy* InPrimitiveSceneProxy);


	protected:

		void OnNewActors(const std::set<Actor*>& NewActors);
		void OnRemoveActor(const Actor* RemovedActor);

		World* m_World;

		std::set<PrimitiveSceneProxy*> m_PrimitiveProxies;

		std::set<StaticMeshComponent*> m_StaticMeshComponents;
		std::set<SceneRenderer*> m_SceneRenderers;

		friend class SceneRenderer;

	private:

	};
}