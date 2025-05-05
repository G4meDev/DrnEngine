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


	protected:

		void RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		void InitResources( ID3D12GraphicsCommandList2* CommandList ) override;
		void UpdateResources( ID3D12GraphicsCommandList2* CommandList ) override;

		PrimitiveComponent* GetPrimitive() override { return m_OwningStaticMeshComponent; };

	private:

		StaticMeshComponent* m_OwningStaticMeshComponent;
		std::vector<AssetHandle<Material>> m_Materials;

		AssetHandle<StaticMesh> m_Mesh;
	};
}