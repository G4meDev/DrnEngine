#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

namespace Drn
{
	struct PrimitiveData
	{
	public:
		PrimitiveData(){};

		Matrix m_LocalToWorld;
		Matrix m_PrevLocalToWorld;
		Guid m_Guid;
	};

	struct Matrix16_4x3
	{
		Matrix16_4x3()
		{}

		Matrix16_4x3(const Matrix InMatrix)
			: m11(InMatrix.m_Matrix._11), m12(InMatrix.m_Matrix._12), m13(InMatrix.m_Matrix._13), m14(InMatrix.m_Matrix._14)
			, m21(InMatrix.m_Matrix._21), m22(InMatrix.m_Matrix._22), m23(InMatrix.m_Matrix._23), m24(InMatrix.m_Matrix._24)
			, m31(InMatrix.m_Matrix._31), m32(InMatrix.m_Matrix._32), m33(InMatrix.m_Matrix._33), m34(InMatrix.m_Matrix._34)
		{}

		Float16 m11, m12, m13, m14;
		Float16 m21, m22, m23, m24;
		Float16 m31, m32, m33, m34;
	};

	class InstancedStaticMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:

		InstancedStaticMeshSceneProxy(InstancedStaticMeshComponent* InInstancedStaticMeshComponent);
		virtual ~InstancedStaticMeshSceneProxy();

		virtual const BoxSphereBounds& GetBounds() override;

	protected:

		void RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
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

		void UpdateBuffers( class D3D12CommandList* CommandList );

		PrimitiveComponent* GetPrimitive() override { return m_OwningInstancedStaticMeshComponent; };
		void BindInstanceBuffers(class D3D12CommandList* CommandList);

	private:

		InstancedStaticMeshComponent* m_OwningInstancedStaticMeshComponent;
		std::vector<MaterialSlot> m_Materials;

		Guid m_Guid;

		AssetHandle<StaticMesh> m_Mesh;

		std::vector<Vector4> OriginRandom;
		std::vector<Matrix16_4x3> LocalToWorldMatrices;

		TRefCountPtr<RenderVertexBuffer> OriginRandomBuffer;
		TRefCountPtr<RenderVertexBuffer> LocalToWorldBuffer;
		TRefCountPtr<RenderVertexBuffer> CustomDataBuffers[NUM_INSTANCED_CUSTOM_DATA];

		PrimitiveData m_PrimitiveData;
		TRefCountPtr<RenderUniformBuffer> PrimitiveBuffer;

		friend class InstancedStaticMeshComponent;
	};
}