#include "DrnPCH.h"
#include "Scene.h"

namespace Drn
{
	Scene::Scene( World* InWorld )
		: m_World(InWorld)
	{
		//for (PrimitiveComponent* Prim : m_World->m_PrimitiveComponents)
		//{
		//	AddPrimitive(Prim);
		//}
	}

	void Scene::Render( dx12lib::CommandList* CommandList )
	{
		for (SceneRenderer* SceneRen : m_SceneRenderers)
		{
			SceneRen->Render(CommandList);
		}
	}

	SceneRenderer* Scene::AllocateSceneRenderer()
	{
		SceneRenderer* NewSceneRenderer = new SceneRenderer(this);
		m_SceneRenderers.insert(NewSceneRenderer);

		return NewSceneRenderer;
	}

	void Scene::RemoveSceneRenderer( SceneRenderer* InSceneRenderer )
	{
		m_SceneRenderers.erase(InSceneRenderer);
	}

	//void Scene::AddPrimitive( PrimitiveComponent* InComponenet )
	//{
	//	m_PrimitiveComponents.push_back(InComponenet);
	//}
	//
	//void Scene::RemovePrimitive( PrimitiveComponent* InComponenet )
	//{
	//	
	//}

}