#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Scene
	{
	public:
		Scene(World* InWorld);
		~Scene();

		inline World* GetWorld() { return m_World; }

		void Render(dx12lib::CommandList* CommandList);

		SceneRenderer* AllocateSceneRenderer();
		void RemoveSceneRenderer(SceneRenderer* InSceneRenderer);
		void RemoveAndInvalidateSceneRenderer(SceneRenderer*& InSceneRenderer);

		void AddStaticMeshCompponent(StaticMeshComponent* InStaticMesh);
		void RemoveStaticMeshCompponent(StaticMeshComponent* InStaticMesh);

	protected:

		World* m_World;

		std::vector<StaticMeshComponent*> m_StaticMeshComponents;
		std::set<SceneRenderer*> m_SceneRenderers;

		friend class SceneRenderer;

	private:

	};
}