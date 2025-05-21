#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

LOG_DECLARE_CATEGORY(LogStaticMeshSceneProxy);

namespace Drn
{
	class StaticMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:

		StaticMeshSceneProxy(StaticMeshComponent* InStaticMeshComponent);
		virtual ~StaticMeshSceneProxy();

		inline bool IsMarkedPendingKill() const { return !m_OwningStaticMeshComponent; }
		inline void MarkPendingKill() { m_OwningStaticMeshComponent = nullptr; }

	protected:

		void RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;

#if WITH_EDITOR
		void RenderHitProxyPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		void RenderSelectionPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		void RenderEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
#endif

		void InitResources( ID3D12GraphicsCommandList2* CommandList ) override;
		void UpdateResources( ID3D12GraphicsCommandList2* CommandList ) override;

		PrimitiveComponent* GetPrimitive() override { return m_OwningStaticMeshComponent; };

	private:

		StaticMeshComponent* m_OwningStaticMeshComponent;
		std::vector<AssetHandle<Material>> m_Materials;

		Guid m_Guid;

		AssetHandle<StaticMesh> m_Mesh;


	};
}