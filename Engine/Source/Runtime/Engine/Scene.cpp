#include "DrnPCH.h"
#include "Scene.h"

namespace Drn
{
	Scene::Scene( World* InWorld )
		: m_World(InWorld)
	{
		for (StaticMeshComponent* Mesh : m_World->m_StaticMeshComponents)
		{
			AddStaticMeshCompponent(Mesh);
		}
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

	void Scene::AddStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		m_StaticMeshComponents.push_back(InStaticMesh);
	}

	void Scene::RemoveStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		
	}

}