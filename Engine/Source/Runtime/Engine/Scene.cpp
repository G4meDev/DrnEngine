#include "DrnPCH.h"
#include "Scene.h"

#include "Runtime/Engine/LightSceneProxy.h"
#include "Runtime/Engine/SkyLightSceneProxy.h"
#include "Runtime/Engine/PostProcessVolume.h"
#include "Runtime/Engine/DecalSceneProxy.h"

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

// --------------------------------------------------------------------------------------

		for (SkyLightSceneProxy* Proxy : m_PendingSkyLightProxies)
		{
			Proxy->Release();
		}

		for (SkyLightSceneProxy* Proxy : m_SkyLightProxies)
		{
			Proxy->Release();
		}

// --------------------------------------------------------------------------------------

		for (DecalSceneProxy* Proxy : m_PendingDecalProxies)
		{
			Proxy->Release();
		}

		for (DecalSceneProxy* Proxy : m_DecalProxies)
		{
			Proxy->Release();
		}


// --------------------------------------------------------------------------------------

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

	void Scene::UpdatePendingProxyAndResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		for (auto it = m_PendingProxies.begin(); it != m_PendingProxies.end(); it++)
		{
			PrimitiveSceneProxy* SceneProxy = *it;
			SceneProxy->InitResources(CommandList->GetD3D12CommandList());
			m_PrimitiveProxies.insert(SceneProxy);
		}
		m_PendingProxies.clear();

		for (auto it = m_PrimitiveProxies.begin(); it != m_PrimitiveProxies.end(); it++)
		{
			PrimitiveSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList->GetD3D12CommandList());
		}

// ----------------------------------------------------------------------------------

		for (auto it = m_PendingLightProxies.begin(); it != m_PendingLightProxies.end(); it++)
		{
			m_LightProxies.insert(*it);
		}
		m_PendingLightProxies.clear();

		for (auto it = m_LightProxies.begin(); it != m_LightProxies.end(); it++)
		{
			LightSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList);
		}

// ----------------------------------------------------------------------------------

		for (auto it = m_PendingSkyLightProxies.begin(); it != m_PendingSkyLightProxies.end(); it++)
		{
			m_SkyLightProxies.insert(*it);
		}
		m_PendingSkyLightProxies.clear();

		for (auto it = m_SkyLightProxies.begin(); it != m_SkyLightProxies.end(); it++)
		{
			SkyLightSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList);
		}

// ----------------------------------------------------------------------------------

		for (auto it = m_PendingPostProcessProxies.begin(); it != m_PendingPostProcessProxies.end(); it++)
		{
			m_PostProcessProxies.insert(*it);
		}
		m_PendingPostProcessProxies.clear();

		for (auto it = m_PostProcessProxies.begin(); it != m_PostProcessProxies.end(); it++)
		{
			PostProcessSceneProxy* Proxy = *it;
			Proxy->UpdateResources();
		}

// ----------------------------------------------------------------------------------

		for (auto it = m_PendingDecalProxies.begin(); it != m_PendingDecalProxies.end(); it++)
		{
			m_DecalProxies.insert(*it);
		}
		m_PendingDecalProxies.clear();

		for (auto it = m_DecalProxies.begin(); it != m_DecalProxies.end(); it++)
		{
			DecalSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList->GetD3D12CommandList());
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

	void Scene::RegisterSkyLightProxy( SkyLightSceneProxy* InLightProxy )
	{
		m_PendingSkyLightProxies.insert(InLightProxy);
	}

	void Scene::UnRegisterSkyLightProxy( SkyLightSceneProxy* InLightProxy )
	{
		if (InLightProxy)
		{
			m_PendingSkyLightProxies.erase(InLightProxy);
			m_SkyLightProxies.erase(InLightProxy);

			InLightProxy->Release();
		}
	}

	void Scene::RegisterPostProcessProxy( class PostProcessSceneProxy* InProxy )
	{
		m_PendingPostProcessProxies.insert(InProxy);
	}

	void Scene::UnRegisterPostProcessProxy( class PostProcessSceneProxy* InProxy )
	{
		m_PendingPostProcessProxies.erase(InProxy);
		m_PostProcessProxies.erase(InProxy);
	}

	void Scene::RegisterDecalProxy( class DecalSceneProxy* InProxy )
	{
		m_PendingDecalProxies.insert(InProxy);
	}

	void Scene::UnRegisterDecalProxy( class DecalSceneProxy* InProxy )
	{
		if (InProxy)
		{
			m_PendingDecalProxies.erase(InProxy);
			m_DecalProxies.erase(InProxy);

			InProxy->Release();
		}
	}

}