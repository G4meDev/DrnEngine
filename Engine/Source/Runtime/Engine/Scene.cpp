#include "DrnPCH.h"
#include "Scene.h"

LOG_DEFINE_CATEGORY( LogScene, "Scene" );

namespace Drn
{
	Scene::Scene( World* InWorld )
		: m_World(InWorld)
	{
		m_World->BindOnNewActors(std::bind(&Scene::OnNewActors, this, std::placeholders::_1));
		m_World->BindOnRemoveActor(std::bind(&Scene::OnRemoveActor, this, std::placeholders::_1));
	}

	Scene::~Scene()
	{
		for (PrimitiveSceneProxy* Proxy : m_PrimitiveProxies)
		{
			delete Proxy;
		}

		for (auto it = m_SceneRenderers.begin(); it != m_SceneRenderers.end();)
		{
			SceneRenderer* SceneRen = *it;
			it = m_SceneRenderers.erase(it);
			delete SceneRen;
		}
	}

	void Scene::Render( dx12lib::CommandList* CommandList )
	{
		SCOPE_STAT(SceneRender);

		InitSceneRender(CommandList);

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

	void Scene::ReleaseSceneRenderer( SceneRenderer*& InSceneRenderer )
	{
		m_SceneRenderers.erase(InSceneRenderer);

		InSceneRenderer->Release();
		InSceneRenderer = nullptr;
	}

// ----------------------------------------------------------------------------

	void Scene::OnNewActors( const std::set<Actor*>& NewActors )
	{
		//for (Actor* actor : NewActors)
		//{
		//	std::vector<StaticMeshComponent*> StaticMeshComponenets;
		//	actor->GetRoot()->GetComponents<StaticMeshComponent>(StaticMeshComponenets, EComponentType::StaticMeshComponent, true);
		//}
	}

	void Scene::OnRemoveActor( const Actor* RemovedActor )
	{
		//std::vector<StaticMeshComponent*> StaticMeshComponenets;
		//RemovedActor->GetRoot()->GetComponents<StaticMeshComponent>(StaticMeshComponenets, EComponentType::StaticMeshComponent, true);
	}

// ----------------------------------------------------------------------------

	void Scene::InitSceneRender(dx12lib::CommandList* CommandList)
	{
		SCOPE_STAT(InitSceneRender);

		for (auto it = m_PendingProxies.begin(); it != m_PendingProxies.end(); it++)
		{
			PrimitiveSceneProxy* SceneProxy = *it;
			SceneProxy->InitResources(CommandList->GetD3D12CommandList().Get());
			m_PrimitiveProxies.insert(SceneProxy);
		}
		m_PendingProxies.clear();

		for (auto it = m_PrimitiveProxies.begin(); it != m_PrimitiveProxies.end(); it++)
		{
			PrimitiveSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList->GetD3D12CommandList().Get());
		}
	}

	void Scene::AddPrimitiveProxy( PrimitiveSceneProxy* InPrimitiveSceneProxy )
	{
		m_PendingProxies.insert(InPrimitiveSceneProxy);
	}

	void Scene::RemovePrimitiveProxy( PrimitiveSceneProxy* InPrimitiveSceneProxy )
	{
		m_PendingProxies.erase(InPrimitiveSceneProxy);
		m_PrimitiveProxies.erase(InPrimitiveSceneProxy);
	}

}