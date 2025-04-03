#include "DrnPCH.h"
#include "Scene.h"

namespace Drn
{
	Scene::Scene( World* InWorld )
		: m_World(InWorld)
	{
		for (Actor* actor : m_World->m_Actors)
		{
			std::vector<StaticMeshComponent*> StaticMeshComponenets;
			actor->GetRoot()->GetComponents<StaticMeshComponent>(StaticMeshComponenets, EComponentType::StaticMeshComponent, true);
		
			for (StaticMeshComponent* Mesh : StaticMeshComponenets)
			{
				AddStaticMeshCompponent(Mesh);
			}
		}

		m_World->BindOnNewActors(std::bind(&Scene::OnNewActors, this, std::placeholders::_1));
		m_World->BindOnRemoveActor(std::bind(&Scene::OnRemoveActor, this, std::placeholders::_1));
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
		m_StaticMeshComponents.insert(InStaticMesh);
	}

	void Scene::RemoveStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		for (auto it = m_StaticMeshComponents.begin(); it != m_StaticMeshComponents.end();)
		{
			if (*it == InStaticMesh)
			{
				it = m_StaticMeshComponents.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

// ----------------------------------------------------------------------------

	void Scene::OnNewActors( const std::set<Actor*>& NewActors )
	{
		for (Actor* actor : NewActors)
		{
			std::vector<StaticMeshComponent*> StaticMeshComponenets;
			actor->GetRoot()->GetComponents<StaticMeshComponent>(StaticMeshComponenets, EComponentType::StaticMeshComponent, true);
		
			for (StaticMeshComponent* Mesh : StaticMeshComponenets)
			{
				AddStaticMeshCompponent(Mesh);
			}
		}
	}

	void Scene::OnRemoveActor( const Actor* RemovedActor )
	{
		std::vector<StaticMeshComponent*> StaticMeshComponenets;
		RemovedActor->GetRoot()->GetComponents<StaticMeshComponent>(StaticMeshComponenets, EComponentType::StaticMeshComponent, true);
	
		for (StaticMeshComponent* Mesh : StaticMeshComponenets)
		{
			RemoveStaticMeshCompponent(Mesh);
		}
	}

}