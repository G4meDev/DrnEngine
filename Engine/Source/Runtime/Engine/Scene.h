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

		void Render(ID3D12GraphicsCommandList2* CommandList);

		SceneRenderer* AllocateSceneRenderer();
		void ReleaseSceneRenderer(SceneRenderer*& InSceneRenderer);

		void InitSceneRender(ID3D12GraphicsCommandList2* CommandList);

		void RegisterPrimitiveProxy(PrimitiveSceneProxy* InPrimitiveSceneProxy);
		void UnRegisterPrimitiveProxy(PrimitiveSceneProxy* InPrimitiveSceneProxy);

	protected:

		World* m_World;

		std::set<PrimitiveSceneProxy*> m_PrimitiveProxies;
		std::set<PrimitiveSceneProxy*> m_EditorPrimitiveProxies;

		std::set<PrimitiveSceneProxy*> m_PendingProxies;

		std::set<SceneRenderer*> m_SceneRenderers;

		friend class SceneRenderer;

	private:

	};
}