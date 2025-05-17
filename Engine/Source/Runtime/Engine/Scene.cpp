#include "DrnPCH.h"
#include "Scene.h"

LOG_DEFINE_CATEGORY( LogScene, "Scene" );

namespace Drn
{
	Scene::Scene( World* InWorld )
		: m_World(InWorld)
	{
	}

	Scene::~Scene()
	{
		for (PrimitiveSceneProxy* Proxy : m_PrimitiveProxies)
		{
			delete Proxy;
		}
		for (PrimitiveSceneProxy* Proxy : m_EditorPrimitiveProxies)
		{
			delete Proxy;
		}
		for (PrimitiveSceneProxy* Proxy : m_PendingProxies)
		{
			delete Proxy;
		}

		for (auto it = m_SceneRenderers.begin(); it != m_SceneRenderers.end();)
		{
			SceneRenderer* SceneRen = *it;
			it = m_SceneRenderers.erase(it);
			SceneRen->Release();
		}
	}

	void Scene::Render( ID3D12GraphicsCommandList2* CommandList )
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

	void Scene::InitSceneRender( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT(InitSceneRender);

		for (auto it = m_PendingProxies.begin(); it != m_PendingProxies.end(); it++)
		{
			PrimitiveSceneProxy* SceneProxy = *it;
			SceneProxy->InitResources(CommandList);
			if (SceneProxy->m_EditorPrimitive)
			{
				m_EditorPrimitiveProxies.insert(SceneProxy);
			}
			else
			{
				m_PrimitiveProxies.insert(SceneProxy);
			}
		}
		m_PendingProxies.clear();

		for (auto it = m_PrimitiveProxies.begin(); it != m_PrimitiveProxies.end(); it++)
		{
			PrimitiveSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList);
		}

		for (auto it = m_EditorPrimitiveProxies.begin(); it != m_EditorPrimitiveProxies.end(); it++)
		{
			PrimitiveSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList);
		}
	}

	void Scene::RegisterPrimitiveProxy( PrimitiveSceneProxy* InPrimitiveSceneProxy )
	{
		m_PendingProxies.insert(InPrimitiveSceneProxy);
	}

	void Scene::UnRegisterPrimitiveProxy( PrimitiveSceneProxy* InPrimitiveSceneProxy )
	{
		if (InPrimitiveSceneProxy)
		{
			m_PendingProxies.erase(InPrimitiveSceneProxy);
			m_PrimitiveProxies.erase(InPrimitiveSceneProxy);
			m_EditorPrimitiveProxies.erase(InPrimitiveSceneProxy);

			//InPrimitiveSceneProxy->ReleaseBufferedResource();
			delete InPrimitiveSceneProxy;
		}
	}

}