#include "DrnPCH.h"
#include "ParticleSpriteSceneProxy.h"
#include "Runtime/Particle/ParticleEmitterType.h"

namespace Drn
{
	ParticleCpuSpriteSceneProxy::ParticleCpuSpriteSceneProxy( ParticleCpuSpriteEmitterInstance* InOwningEmitter )
		: PrimitiveSceneProxy(InOwningEmitter->Component)
		, OwningEmitter(InOwningEmitter )
		, Guid(InOwningEmitter->Component->GetGuid())
		, ActiveParticles(0)
		, MaxParticles(0)
	{
#if WITH_EDITOR
		m_EditorPrimitive = InOwningEmitter->Component->IsEditorPrimitive();
		m_Selectable = InOwningEmitter->Component->IsSelectable();
#endif

		MinDrawDistance = InOwningEmitter->Component->GetMinDrawDistance();
		MaxDrawDistance = InOwningEmitter->Component->GetMaxDrawDistance();
	}

	ParticleCpuSpriteSceneProxy::~ParticleCpuSpriteSceneProxy()
	{
		
	}

	const BoxSphereBounds& ParticleCpuSpriteSceneProxy::GetBounds()
	{
		drn_check(OwningEmitter);
		return OwningEmitter->GetBoundingBox();
	}

	void ParticleCpuSpriteSceneProxy::RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderTranslucencyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderDistortionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderShadowPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderDecalPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

#if WITH_EDITOR
	void ParticleCpuSpriteSceneProxy::RenderHitProxyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}
#endif

	void ParticleCpuSpriteSceneProxy::InitResources( class D3D12CommandList* CommandList )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::UpdateResources( class D3D12CommandList* CommandList )
	{
		UpdateMaterial(CommandList);
		UpdateBuffers(CommandList);
	}

	void ParticleCpuSpriteSceneProxy::UpdateMaterial( class D3D12CommandList* CommandList )
	{
		drn_check(OwningEmitter && OwningEmitter->Emitter && OwningEmitter->Emitter->GetEmitterType());
		ParticleEmitterCpuSpriteType* EmitterSpriteType = OwningEmitter->Emitter->GetEmitterType()->GetType() == EEmitterType::Sprite_Cpu ? 
			static_cast<ParticleEmitterCpuSpriteType*>(OwningEmitter->Emitter->GetEmitterType()) : nullptr;

		SpriteMaterial = EmitterSpriteType->SpriteMaterial;
		SpriteMaterial.LoadChecked();
		if (!SpriteMaterial.IsValid())
		{
			//LOG(LogStaticMeshSceneProxy, Error, "Material is invalid. Using default material.");

			SpriteMaterial = AssetHandle<Material>(DEFAULT_MATERIAL_PATH);
			SpriteMaterial.Load();
		}

		SpriteMaterial.GetMaterialInterface()->UploadResources(CommandList);
	}

	void ParticleCpuSpriteSceneProxy::UpdateBuffers( class D3D12CommandList* CommandList )
	{
		drn_check(OwningEmitter);

		if (MaxParticles < OwningEmitter->MaxActiveParticles)
		{
			ParticlesInstanceData.resize(OwningEmitter->MaxActiveParticles);
		}

		ActiveParticles = OwningEmitter->ActiveParticles;
		MaxParticles = OwningEmitter->MaxActiveParticles;

		ParticleData.m_Guid = Guid;
		ParticleData.m_LocalToWorld = OwningEmitter->EmitterToSimulation;
		ParticleBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(ParticleSpriteData), EUniformBufferUsage::MultiFrame, &ParticleData);

		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * OwningEmitter->ParticleIndices[i]);

			ParticlesInstanceData[i].OldPosition = Particle.OldLocation;
			ParticlesInstanceData[i].Position = Particle.Location;
			ParticlesInstanceData[i].Color = Particle.Color;
			ParticlesInstanceData[i].RelativeTime = Particle.RelativeTime;
			ParticlesInstanceData[i].Size = Vector2(Particle.Size.X, Particle.Size.Y);
			ParticlesInstanceData[i].Rotation = Particle.Rotation;
			ParticlesInstanceData[i].ParticleId = Particle.Flags & EParticleStates::STATE_CounterMask;
			ParticlesInstanceData[i].SubImageIndex = 0;
		}

		if (ActiveParticles > 0)
		{
			drn_check(MaxParticles > 0);
			drn_check(MaxParticles >= ActiveParticles);

			{
				uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Dynamic;
				RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, ParticlesInstanceData.data(), ClearValueBinding::Black, "ParticlesInstanceData");
				ParticlesInstanceBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, MaxParticles * sizeof(ParticleSpriteVertex), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
			}
		}
		else
		{
			ParticlesInstanceBuffer = nullptr;
			DynamicBuffer = nullptr;
		}
	}

	void ParticleCpuSpriteSceneProxy::BindInstanceBuffers( class D3D12CommandList* CommandList )
	{
		CommandList->SetStreamSource(8, ParticlesInstanceBuffer, 0);
		CommandList->SetStreamSource(9, DynamicBuffer, 0);
	}

}  // namespace Drn