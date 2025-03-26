#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Scene
	{
	public:
		Scene(World* InWorld);

		inline World* GetWorld() { return m_World; }

		void Render(dx12lib::CommandList* CommandList);

		SceneRenderer* AllocateSceneRenderer();
		void RemoveSceneRenderer(SceneRenderer* InSceneRenderer);

		//void AddPrimitive(PrimitiveComponent* InComponenet);
		//void RemovePrimitive(PrimitiveComponent* InComponenet);

	protected:

		World* m_World;

		std::vector<StaticMeshComponent*> m_PrimitiveComponents;
		std::set<SceneRenderer*> m_SceneRenderers;

	private:

	};
}