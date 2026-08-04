#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct ParticleSpriteData
	{
	public:
		ParticleSpriteData(){};

		Matrix m_LocalToWorld;
		Matrix m_PrevLocalToWorld;
		HitProxyData m_HitProxyData;

		Vector NormalsSphereCenter;
		float Unused_1;

		Vector NormalsCylinderDirection;
		float Unused_2;

		Vector EmitterForward;
		float Unused_3;

		Vector EmitterUp;
		float Unused_4;

		Vector EmitterRight;
		float Unused_5;
	};

	class ParticleCpuSpriteSceneProxy : public PrimitiveSceneProxy
	{
	public:
		ParticleCpuSpriteSceneProxy(ParticleCpuSpriteEmitterInstance* InOwningEmitter);
		virtual ~ParticleCpuSpriteSceneProxy();

		virtual const BoxSphereBounds& GetBounds() override;
		virtual PrimitiveComponent* GetPrimitive() override{ return OwningEmitter->Component; }

		virtual void OnRegister(Scene* InScene) override;
		virtual void OnRemove(Scene* InScene) override;

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

		void UpdateMaterial( class D3D12CommandList* CommandList );
		void UpdateBuffers( class D3D12CommandList* CommandList );

		void BindInstanceBuffers(class D3D12CommandList* CommandList);

	private:

		ParticleCpuSpriteEmitterInstance* OwningEmitter;
		MaterialSlot SpriteMaterial;
		HitProxyData m_HitProxyData;

		int32 ActiveParticles;
		int32 MaxParticles;
		EParticleSortMode SortMode;
		std::vector<uint16> SortedInidices;

		std::vector<ParticleSpriteVertex> ParticlesInstanceData;
		std::vector<MeshParticleInstanceVertexDynamicParameter> Dynamics;

		TRefCountPtr<RenderVertexBuffer> ParticlesInstanceBuffer;
		TRefCountPtr<RenderVertexBuffer> DynamicBuffer;

		ParticleSpriteData ParticleData;
		TRefCountPtr<RenderUniformBuffer> ParticleBuffer;

		class ParticleLightSceneProxy* LightProxy;

// -------------------------------------------------------------------------------------------

		friend class ParticleEmitterInstance;
	};
}