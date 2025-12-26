#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

LOG_DECLARE_CATEGORY(LogStaticMeshSceneProxy);

namespace Drn
{
	struct PrimitiveBuffer
	{
	public:
		PrimitiveBuffer(){};

		Matrix m_LocalToWorld;
		Matrix m_LocalToProjection;
		Guid m_Guid;
		Matrix m_PrevLocalToWorld;
		Matrix m_PrevLocalToProjection;
	};

	class StaticMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:

		StaticMeshSceneProxy(StaticMeshComponent* InStaticMeshComponent);
		virtual ~StaticMeshSceneProxy();

		inline bool IsMarkedPendingKill() const { return !m_OwningStaticMeshComponent; }
		inline void MarkPendingKill() { m_OwningStaticMeshComponent = nullptr; }

	protected:

		void RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy, uint32 ShadowBufferIndex) override;
		virtual void RenderDecalPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) override;


#if WITH_EDITOR
		void RenderHitProxyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
#endif

		void InitResources( class D3D12CommandList* CommandList ) override;
		void UpdateResources( class D3D12CommandList* CommandList ) override;

		PrimitiveComponent* GetPrimitive() override { return m_OwningStaticMeshComponent; };

	private:

		StaticMeshComponent* m_OwningStaticMeshComponent;
		std::vector<AssetHandle<Material>> m_Materials;

		Guid m_Guid;

		AssetHandle<StaticMesh> m_Mesh;

		PrimitiveBuffer m_PrimitiveBuffer;
	};
}