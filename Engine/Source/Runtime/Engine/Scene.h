#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogScene);

class ID3D12GraphicsCommandList2;

namespace Drn
{
	class PrimitiveSceneProxy;

	class Scene
	{
	public:
		Scene(World* InWorld);
		~Scene();

		inline void Release() { delete this; }

		inline World* GetWorld() { return m_World; }

		SceneRenderer* AllocateSceneRenderer();
		void ReleaseSceneRenderer(SceneRenderer*& InSceneRenderer);

		void UpdatePendingProxyAndResources(class D3D12CommandList* CommandList);

		void RegisterPrimitiveProxy(PrimitiveSceneProxy* InPrimitiveSceneProxy);

		void RegisterLightProxy( class LightSceneProxy* InLightProxy );
		void UnRegisterLightProxy( class LightSceneProxy* InLightProxy );

		void RegisterSkyLightProxy( class SkyLightSceneProxy* InLightProxy );
		void UnRegisterSkyLightProxy( class SkyLightSceneProxy* InLightProxy );

		void RegisterPostProcessProxy( class PostProcessSceneProxy* InProxy );
		void UnRegisterPostProcessProxy( class PostProcessSceneProxy* InProxy );

		void RegisterDecalProxy( class DecalSceneProxy* InProxy );
		void UnRegisterDecalProxy( class DecalSceneProxy* InProxy );

		inline const std::vector<PrimitiveSceneProxy*>& GetPrimitiveProxies() { return m_PrimitiveProxies; };
		inline const std::set<LightSceneProxy*>& GetLightProxies() const { return m_LightProxies; };

	protected:

		World* m_World;

		std::vector<PrimitiveSceneProxy*> m_PrimitiveProxies;
		std::vector<PrimitiveSceneProxy*> m_PendingProxies;

		std::set<class LightSceneProxy*> m_LightProxies;
		std::set<class LightSceneProxy*> m_PendingLightProxies;

		std::set<class SkyLightSceneProxy*> m_SkyLightProxies;
		std::set<class SkyLightSceneProxy*> m_PendingSkyLightProxies;

		std::set<class PostProcessSceneProxy*> m_PostProcessProxies;
		std::set<class PostProcessSceneProxy*> m_PendingPostProcessProxies;

		std::set<class DecalSceneProxy*> m_DecalProxies;
		std::set<class DecalSceneProxy*> m_PendingDecalProxies;

		std::set<SceneRenderer*> m_SceneRenderers;

		friend class ReflectionEnvironmentBuffer;
		friend class SceneRenderer;
		friend class Renderer;

	private:

	};
}