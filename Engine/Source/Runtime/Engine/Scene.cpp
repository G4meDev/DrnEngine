#include "DrnPCH.h"
#include "Scene.h"

#include "Runtime/Engine/LightSceneProxy.h"

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
		for (PrimitiveSceneProxy* Proxy : m_PendingProxies)
		{
			delete Proxy;
		}

		for (LightSceneProxy* Proxy : m_PendingLightProxies)
		{
			Proxy->Release();
		}

		for (LightSceneProxy* Proxy : m_LightProxies)
		{
			Proxy->Release();
		}

		for (auto it = m_SceneRenderers.begin(); it != m_SceneRenderers.end();)
		{
			SceneRenderer* SceneRen = *it;
			it = m_SceneRenderers.erase(it);
			SceneRen->Release();
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

	void Scene::UpdatePendingProxyAndResources( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT();

		for (auto it = m_PendingProxies.begin(); it != m_PendingProxies.end(); it++)
		{
			PrimitiveSceneProxy* SceneProxy = *it;
			SceneProxy->InitResources(CommandList);
			m_PrimitiveProxies.insert(SceneProxy);
		}
		m_PendingProxies.clear();

		for (auto it = m_PrimitiveProxies.begin(); it != m_PrimitiveProxies.end(); it++)
		{
			PrimitiveSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList);
		}

// ----------------------------------------------------------------------------------

		for (auto it = m_PendingLightProxies.begin(); it != m_PendingLightProxies.end(); it++)
		{
			m_LightProxies.insert(*it);
		}
		m_PendingLightProxies.clear();
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

			//InPrimitiveSceneProxy->ReleaseBufferedResource();
			delete InPrimitiveSceneProxy;
		}
	}

	void Scene::RegisterLightProxy( LightSceneProxy* InLightProxy )
	{
		m_PendingLightProxies.insert(InLightProxy);
	}

	void Scene::UnRegisterLightProxy( LightSceneProxy* InLightProxy )
	{
		if (InLightProxy)
		{
			m_PendingLightProxies.erase(InLightProxy);
			m_LightProxies.erase(InLightProxy);

			InLightProxy->Release();
		}
	}

}