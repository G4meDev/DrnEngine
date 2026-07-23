#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct ParticleMeshData
	{
	public:
		ParticleMeshData(){};

		Matrix m_LocalToWorld;
		Matrix m_PrevLocalToWorld;
		Guid m_Guid;
	};

	class ParticleMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:
		ParticleMeshSceneProxy(ParticleMeshEmitterInstance* InOwningEmitter);
		virtual ~ParticleMeshSceneProxy();

		virtual const BoxSphereBounds& GetBounds() override;
		virtual PrimitiveComponent* GetPrimitive() override{ return OwningEmitter->Component; }

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

		void UpdateMeshAndMaterials( class D3D12CommandList* CommandList );
		void UpdateBuffers( class D3D12CommandList* CommandList );

		void BindInstanceBuffers(class D3D12CommandList* CommandList);

	private:

		ParticleMeshEmitterInstance* OwningEmitter;
		AssetHandle<StaticMesh> Mesh;
		std::vector<MaterialSlot> Materials;
		Guid Guid;

		int32 ActiveParticles;
		int32 MaxParticles;

		std::vector<MeshParticleInstanceVertex> ParticlesInstanceData;
		std::vector<MeshParticleInstanceVertexPrevTransform> PrevTransforms;
		std::vector<MeshParticleInstanceVertexDynamicParameter> Dynamics;

		TRefCountPtr<RenderVertexBuffer> ParticlesInstanceBuffer;
		TRefCountPtr<RenderVertexBuffer> PrevTransformBuffer;
		TRefCountPtr<RenderVertexBuffer> DynamicBuffer;

		ParticleMeshData ParticleData;
		TRefCountPtr<RenderUniformBuffer> ParticleBuffer;

		friend class ParticleEmitterInstance;
	};
}