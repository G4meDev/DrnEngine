#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

LOG_DECLARE_CATEGORY(LogSkeletalMeshSceneProxy);

namespace Drn
{
	struct PrimitiveData
	{
	public:
		PrimitiveData(){};

		Matrix m_LocalToWorld;
		Matrix m_PrevLocalToWorld;
		HitProxyData m_HitProxyData;

		//Matrix BoneMatrices[MAX_BONES];
	};

	class SkeletalMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:

		SkeletalMeshSceneProxy( SkeletalMeshComponent* InStaticMeshComponent );
		virtual ~SkeletalMeshSceneProxy();

		virtual const BoxSphereBounds& GetBounds() override;

	protected:

		void RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderTranslucencyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderDistortionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy) override;
		virtual void RenderDecalPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) override;


#if WITH_EDITOR
		void RenderHitProxyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		void RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
#endif

		void InitResources( class D3D12CommandList* CommandList ) override;
		void UpdateResources( class D3D12CommandList* CommandList ) override;

		void UpdatePrimitiveBuffer(class D3D12CommandList* CommandList);


		PrimitiveComponent* GetPrimitive() override { return m_OwningSkeletalMeshComponent; };

	private:

		SkeletalMeshComponent* m_OwningSkeletalMeshComponent;
		std::vector<MaterialSlot> m_Materials;

		HitProxyData m_HitProxyData;

		AssetHandle<SkeletalMesh> m_Mesh;

		PrimitiveData m_PrimitiveData;
		TRefCountPtr<RenderUniformBuffer> PrimitiveBuffer;

		bool bWasDirty;

		friend class SkeletalMeshComponent;
	};
}