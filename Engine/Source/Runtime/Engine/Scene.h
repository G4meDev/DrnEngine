#pragma once

#include "ForwardTypes.h"
#include "Runtime/Containers/BitArray.h"

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
		void RegisterSkyLightProxy( class SkyLightSceneProxy* InLightProxy );

		void RegisterPostProcessProxy( class PostProcessSceneProxy* InProxy );
		void UnRegisterPostProcessProxy( class PostProcessSceneProxy* InProxy );

		void RegisterDecalProxy( class DecalSceneProxy* InProxy );
		void UnRegisterDecalProxy( class DecalSceneProxy* InProxy );

		inline const std::vector<PrimitiveSceneProxy*>& GetPrimitiveProxies() { return m_PrimitiveProxies; };
		inline const std::vector<LightSceneProxy*>& GetLightProxies() { return m_LightProxies; };

		inline const BitArray& GetStaticPrimitiveProxiesMap() { return StaticPrimitiveMap; };
		inline const BitArray& GetDynamicPrimitiveProxiesMap() { return DynamicPrimitiveMap; };

	protected:

		World* m_World;

		std::vector<PrimitiveSceneProxy*> m_PrimitiveProxies;
		std::vector<PrimitiveSceneProxy*> m_PendingProxies;

		std::vector<class LightSceneProxy*> m_LightProxies;
		std::vector<class LightSceneProxy*> m_PendingLightProxies;

		std::vector<class SkyLightSceneProxy*> m_SkyLightProxies;
		std::vector<class SkyLightSceneProxy*> m_PendingSkyLightProxies;

		std::set<class PostProcessSceneProxy*> m_PostProcessProxies;
		std::set<class PostProcessSceneProxy*> m_PendingPostProcessProxies;

		std::set<class DecalSceneProxy*> m_DecalProxies;
		std::set<class DecalSceneProxy*> m_PendingDecalProxies;

		std::set<SceneRenderer*> m_SceneRenderers;

		// TODO: merge into one map with set and unset bit iterator
		BitArray StaticPrimitiveMap;
		BitArray DynamicPrimitiveMap;

		friend class ReflectionEnvironmentBuffer;
		friend class SceneRenderer;
		friend class Renderer;

	private:

#if WITH_EDITOR
		struct ReflectionCaptureEvent
		{
			SceneRenderer* CaptureSceneRenderer;
			TRefCountPtr<class RenderTexture2D> Target;
			uint64 CaptureFenceValue;
			int32 FaceIndex;
		};
		std::vector<ReflectionCaptureEvent> ReflectionCaptureEvents;
#endif
	};
}