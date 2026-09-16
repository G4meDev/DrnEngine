#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

namespace Drn
{
	struct MeshSectionRenderData
	{
		TRefCountPtr<class StaticMeshVertexBuffer> m_VertexBuffer;
		TRefCountPtr<RenderIndexBuffer> m_IndexBuffer;
		MaterialSlot m_Material;

		uint32 VertexCount;
		uint32 PrimitiveCount;

		bool bVisible;

		inline bool IsValid() const
		{
			return m_VertexBuffer && m_IndexBuffer && m_Material.IsValid();
		}

		inline void BindAndDraw(D3D12CommandList* CommandList) const
		{
			m_VertexBuffer->Bind(CommandList);
			CommandList->DrawIndexedPrimitive(m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, 1);
		}

	};

	struct PrimitiveData
	{
	public:
		PrimitiveData(){};

		Matrix m_LocalToWorld;
		Matrix m_PrevLocalToWorld;
		HitProxyData m_HitProxyData;
	};

	class DynamicMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:

		DynamicMeshSceneProxy( DynamicMeshComponent* InDynamicMeshComponent );
		virtual ~DynamicMeshSceneProxy();

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

		void UpdateDynamicMeshBuffer(class D3D12CommandList* CommandList);
		void UpdatePrimitiveBuffer(class D3D12CommandList* CommandList);

		PrimitiveComponent* GetPrimitive() override;

	private:

		DynamicMeshComponent* m_OwningDynamicMeshComponent;
		std::vector<MeshSectionRenderData> SectionsRenderData;
		
		HitProxyData m_HitProxyData;

		PrimitiveData m_PrimitiveData;
		TRefCountPtr<RenderUniformBuffer> PrimitiveBuffer;

		bool bWasDirty;

		friend class DynamicMeshComponent;
	};
}