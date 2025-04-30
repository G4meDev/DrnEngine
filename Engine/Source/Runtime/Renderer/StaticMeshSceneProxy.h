#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

namespace Drn
{
	class StaticMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:

		StaticMeshSceneProxy(StaticMeshComponent* InStaticMeshComponent);
		virtual ~StaticMeshSceneProxy();


	protected:

		void RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer ) override;
		void InitResources( dx12lib::CommandList* CommandList ) override;
		void UpdateResources( dx12lib::CommandList* CommandList ) override;

		PrimitiveComponent* GetPrimitive() override { return m_OwningStaticMeshComponent; };

	private:

		StaticMeshComponent* m_OwningStaticMeshComponent;

		// TODO: Delete
		AssetHandle<Material> m_TempMat;
	};
}