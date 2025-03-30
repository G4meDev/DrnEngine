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

	Scene::~Scene()
	{
		for (auto it = m_SceneRenderers.begin(); it != m_SceneRenderers.end();)
		{
			SceneRenderer* SceneRen = *it;
			it = m_SceneRenderers.erase(it);
			delete SceneRen;
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

	void Scene::RemoveAndInvalidateSceneRenderer( SceneRenderer*& InSceneRenderer )
	{
		RemoveSceneRenderer(InSceneRenderer);
		delete InSceneRenderer;
		InSceneRenderer = nullptr;
	}

	void Scene::AddStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		m_StaticMeshComponents.push_back(InStaticMesh);
	}

	void Scene::RemoveStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		
	}

	void Scene::AddCameraComponent( CameraComponent* InCamera )
	{
		m_CameraComponents.push_back(InCamera);
	}

	void Scene::RemoveCameraComponent( CameraComponent* InCamera )
	{
		
	}

}