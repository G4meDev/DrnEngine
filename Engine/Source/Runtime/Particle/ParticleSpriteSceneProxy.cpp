#include "DrnPCH.h"
#include "ParticleSpriteSceneProxy.h"
#include "Runtime/Particle/ParticleEmitterType.h"
#include "Runtime/Particle/ParticleLightSceneProxy.h"

namespace Drn
{
	ParticleCpuSpriteSceneProxy::ParticleCpuSpriteSceneProxy( ParticleCpuSpriteEmitterInstance* InOwningEmitter )
		: PrimitiveSceneProxy(InOwningEmitter->Component)
		, OwningEmitter(InOwningEmitter)
		, LightProxy(nullptr)
		, m_HitProxyData(InOwningEmitter->Component)
		, SortMode(InOwningEmitter->Emitter->SortMode)
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

		Bounds = OwningEmitter->GetBoundingBox();
		return Bounds;
	}

	void ParticleCpuSpriteSceneProxy::OnRegister( Scene* InScene )
	{
		if (OwningEmitter->Emitter->IsLightActive())
		{
			drn_check(!LightProxy);
			LightProxy = new ParticleLightSceneProxy(OwningEmitter);
			InScene->RegisterParticleLightProxy(LightProxy);
		}
	}

	void ParticleCpuSpriteSceneProxy::OnRemove( Scene* InScene )
	{
		if (LightProxy)
		{
			InScene->UnRegisterParticleLightProxy(LightProxy);
			delete LightProxy;
			LightProxy = nullptr;
		}
	}

	void ParticleCpuSpriteSceneProxy::RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleCpuSpriteSceneProxy::RenderTranslucencyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (ActiveParticles > 0)
		{
			MaterialShader* MatShader = SpriteMaterial.GetParentMaterial()->GetShaderParameters().bIsUsedWithParticleSprite && SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasTranslucencyPass
				? SpriteMaterial.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::ParticleSprite, EMaterialStage::Translucensy)
				: nullptr;

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(SpriteMaterial.GetMaterialName().c_str());
		
				MatShader->Bind(CommandList);
				SpriteMaterial.GetMaterialInterface()->BindResources(CommandList);
		
				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(ParticleBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
				CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
				BindInstanceBuffers(CommandList);
				CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_ParticleSprite->m_IndexBuffer, 0, 0,
					CommonResources::Get()->m_ParticleSprite->VertexCount, 0, CommonResources::Get()->m_ParticleSprite->PrimitiveCount, ActiveParticles);
			}
		}
	}

	void ParticleCpuSpriteSceneProxy::RenderDistortionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (ActiveParticles > 0)
		{
			MaterialShader* MatShader = SpriteMaterial.GetParentMaterial()->GetShaderParameters().bIsUsedWithParticleSprite && SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasDistortionPass
				? SpriteMaterial.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::ParticleSprite, EMaterialStage::Distortion)
				: nullptr;

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(SpriteMaterial.GetMaterialName().c_str());
		
				MatShader->Bind(CommandList);
				SpriteMaterial.GetMaterialInterface()->BindResources(CommandList);
		
				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(ParticleBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
				CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
				BindInstanceBuffers(CommandList);
				CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_ParticleSprite->m_IndexBuffer, 0, 0,
					CommonResources::Get()->m_ParticleSprite->VertexCount, 0, CommonResources::Get()->m_ParticleSprite->PrimitiveCount, ActiveParticles);
			}
		}
	}

	void ParticleCpuSpriteSceneProxy::RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (ActiveParticles > 0)
		{
			MaterialShader* MatShader = SpriteMaterial.GetParentMaterial()->GetShaderParameters().bIsUsedWithParticleSprite && SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasMainPass
				? SpriteMaterial.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::ParticleSprite, EMaterialStage::Main)
				: nullptr;

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(SpriteMaterial.GetMaterialName().c_str());
		
				MatShader->Bind(CommandList);
				SpriteMaterial.GetMaterialInterface()->BindResources(CommandList);
		
				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(ParticleBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
				CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
				BindInstanceBuffers(CommandList);

				CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
				BindInstanceBuffers(CommandList);
				CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_ParticleSprite->m_IndexBuffer, 0, 0,
					CommonResources::Get()->m_ParticleSprite->VertexCount, 0, CommonResources::Get()->m_ParticleSprite->PrimitiveCount, ActiveParticles);
			}
		}
	}

	void ParticleCpuSpriteSceneProxy::RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (ActiveParticles > 0)
		{
			MaterialShader* MatShader = nullptr;
			if (SpriteMaterial.GetParentMaterial()->GetShaderParameters().bIsUsedWithParticleSprite && SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasPrepass)
			{
				MatShader = SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasCustomPrepass
					? SpriteMaterial.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::ParticleSprite, EMaterialStage::Prepass)
					: CommonResources::Get()->m_PositionOnlyMaterialShaders.GetShader(VertexFactoryType::ParticleSprite, SpriteMaterial.GetParentMaterial()->IsTwoSided());
			}

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(SpriteMaterial.GetMaterialName().c_str());
		
				MatShader->Bind(CommandList);
				SpriteMaterial.GetMaterialInterface()->BindResources(CommandList);
		
				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(ParticleBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
				BindInstanceBuffers(CommandList);
				CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_ParticleSprite->m_IndexBuffer, 0, 0,
					CommonResources::Get()->m_ParticleSprite->VertexCount, 0, CommonResources::Get()->m_ParticleSprite->PrimitiveCount, ActiveParticles);
			}
		}
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
		SCOPE_STAT("HitProxyParticle");
		
		if (ActiveParticles == 0 || !m_Selectable)
		{
			return;
		}
		
		MaterialShader* MatShader = SpriteMaterial.GetParentMaterial()->GetShaderParameters().bIsUsedWithParticleSprite && SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasHitProxyPass
			? SpriteMaterial.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::ParticleSprite, EMaterialStage::Hitproxy)
			: nullptr;

		if (MatShader)
		{
			SCOPE_STAT_DYNAMIC(SpriteMaterial.GetMaterialName().c_str());

			MatShader->Bind(CommandList);
			SpriteMaterial.GetMaterialInterface()->BindResources(CommandList);
				
			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
			CommandList->SetGraphicRootConstant(ParticleBuffer->GetViewIndex(), 1);
			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
			CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
			BindInstanceBuffers(CommandList);
			CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_ParticleSprite->m_IndexBuffer, 0, 0,
				CommonResources::Get()->m_ParticleSprite->VertexCount, 0, CommonResources::Get()->m_ParticleSprite->PrimitiveCount, ActiveParticles);
		}
	}

	void ParticleCpuSpriteSceneProxy::RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (ActiveParticles == 0 || !m_SelectedInEditor)
			return;
		
		MaterialShader* MatShader = SpriteMaterial.GetParentMaterial()->GetShaderParameters().bIsUsedWithParticleSprite && SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasEditorSelectionPass
			? SpriteMaterial.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::ParticleSprite, EMaterialStage::EditorSelection)
			: nullptr;

		if (MatShader)
		{
			SCOPE_STAT_DYNAMIC(SpriteMaterial.GetMaterialName().c_str());

			MatShader->Bind(CommandList);
			SpriteMaterial.GetMaterialInterface()->BindResources(CommandList);

			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
			CommandList->SetGraphicRootConstant(ParticleBuffer->GetViewIndex(), 1);
			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

			CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
			BindInstanceBuffers(CommandList);
			CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_ParticleSprite->m_IndexBuffer, 0, 0,
				CommonResources::Get()->m_ParticleSprite->VertexCount, 0, CommonResources::Get()->m_ParticleSprite->PrimitiveCount, ActiveParticles);
		}
	}

	void ParticleCpuSpriteSceneProxy::RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (ActiveParticles == 0 || !m_EditorPrimitive)
		{
			return;
		}

		MaterialShader* MatShader = SpriteMaterial.GetParentMaterial()->GetShaderParameters().bIsUsedWithParticleSprite && SpriteMaterial.GetParentMaterial()->GetShaderParameters().bHasEditorPrimitivePass
			? SpriteMaterial.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::ParticleSprite, EMaterialStage::EditorPrimitive)
			: nullptr;

		if (MatShader)
		{
			SCOPE_STAT_DYNAMIC(SpriteMaterial.GetMaterialName().c_str());
		
			MatShader->Bind(CommandList);
			SpriteMaterial.GetMaterialInterface()->BindResources(CommandList);

			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
			CommandList->SetGraphicRootConstant(ParticleBuffer->GetViewIndex(), 1);
			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
			CommonResources::Get()->m_ParticleSprite->Bind(CommandList);
			BindInstanceBuffers(CommandList);
			CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_ParticleSprite->m_IndexBuffer, 0, 0,
				CommonResources::Get()->m_ParticleSprite->VertexCount, 0, CommonResources::Get()->m_ParticleSprite->PrimitiveCount, ActiveParticles);
		}
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

			if (SortMode != EParticleSortMode::None)
			{
				SortedInidices.resize(OwningEmitter->MaxActiveParticles);
			}
		}

		ActiveParticles = OwningEmitter->ActiveParticles;
		MaxParticles = OwningEmitter->MaxActiveParticles;

		Matrix EmitterToWorld = OwningEmitter->EmitterToSimulation * OwningEmitter->SimulationToWorld;

		ParticleData.m_HitProxyData = m_HitProxyData;
		ParticleData.m_LocalToWorld = OwningEmitter->SimulationToWorld;
		ParticleData.NormalsSphereCenter = EmitterToWorld.TransformPosition(OwningEmitter->Emitter->NormalsSphereCenter);
		ParticleData.NormalsCylinderDirection = EmitterToWorld.TransformVector(OwningEmitter->Emitter->NormalsCylinderDirection);

		Matrix EmitterToComponent = Transform(OwningEmitter->Emitter->Origin, OwningEmitter->Emitter->Rotation);
		Matrix LocalToEmitter = EmitterToComponent * OwningEmitter->Component->GetWorldTransform().ToMatrixNoScale();

		ParticleData.EmitterForward	= LocalToEmitter.TransformVector(Vector::ForwardVector).GetSafeNormal();
		ParticleData.EmitterUp		= LocalToEmitter.TransformVector(Vector::UpVector).GetSafeNormal();
		ParticleData.EmitterRight	= LocalToEmitter.TransformVector(Vector::RightVector).GetSafeNormal();

		ParticleBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(ParticleSpriteData), EUniformBufferUsage::MultiFrame, &ParticleData);

		uint16* ParticleIndices = OwningEmitter->ParticleIndices;
		if (SortMode != EParticleSortMode::None)
		{
			memcpy(SortedInidices.data(), OwningEmitter->ParticleIndices, sizeof(uint16) * ActiveParticles);

			if (SortMode == EParticleSortMode::ViewProjDepth)
			{
				//Vector CameraLocation = OwningEmitter->Component->GetWorld()->GetPlayerWorldView().Location;
				//
				//std::sort(SortedInidices.begin(), SortedInidices.begin() + ActiveParticles,
				//	[&](uint16 A, uint16 B)
				//	{
				//		DECLARE_PARTICLE(ParticleA, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[A]);
				//		DECLARE_PARTICLE(ParticleB, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[B]);
				//
				//		float DistA = Vector::DistSquared(OwningEmitter->SimulationToWorld.TransformPosition(ParticleA.Location), CameraLocation);
				//		float DistB = Vector::DistSquared(OwningEmitter->SimulationToWorld.TransformPosition(ParticleB.Location), CameraLocation);
				//
				//		return DistA > DistB;
				//	});
			}

			else if (SortMode == EParticleSortMode::DistanceToView)
			{
				Vector CameraLocation = OwningEmitter->Component->GetWorld()->GetPlayerWorldView().Location;

				std::sort(SortedInidices.begin(), SortedInidices.begin() + ActiveParticles,
					[&](uint16 A, uint16 B)
					{
						DECLARE_PARTICLE(ParticleA, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[A]);
						DECLARE_PARTICLE(ParticleB, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[B]);

						float DistA = Vector::DistSquared(OwningEmitter->SimulationToWorld.TransformPosition(ParticleA.Location), CameraLocation);
						float DistB = Vector::DistSquared(OwningEmitter->SimulationToWorld.TransformPosition(ParticleB.Location), CameraLocation);

						return DistA > DistB;
					});
			}

			else if (SortMode == EParticleSortMode::Age_OldestFirst)
			{
				std::sort(SortedInidices.begin(), SortedInidices.begin() + ActiveParticles,
					[&](uint16 A, uint16 B)
					{
						DECLARE_PARTICLE(ParticleA, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[A]);
						DECLARE_PARTICLE(ParticleB, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[B]);

						return ParticleA.RelativeTime > ParticleB.RelativeTime;
					});
			}

			else if (SortMode == EParticleSortMode::Age_NewestFirst)
			{
				std::sort(SortedInidices.begin(), SortedInidices.begin() + ActiveParticles,
					[&](uint16 A, uint16 B)
					{
						DECLARE_PARTICLE(ParticleA, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[A]);
						DECLARE_PARTICLE(ParticleB, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[B]);

						return ParticleA.RelativeTime < ParticleB.RelativeTime;
					});
			}

			ParticleIndices = SortedInidices.data();
		}

		const Vector Scale = OwningEmitter->Component->GetWorldScale();

		const bool bHasSubuv = OwningEmitter->Emitter->bHasSubuv;
		const int32 SubuvOffset = OwningEmitter->Emitter->GetSubuvOffset();

		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * ParticleIndices[i]);

			ParticlesInstanceData[i].OldPosition = Particle.OldLocation;
			ParticlesInstanceData[i].Position = Particle.Location;
			ParticlesInstanceData[i].Color = Particle.Color;
			ParticlesInstanceData[i].RelativeTime = Particle.RelativeTime;
			//ParticlesInstanceData[i].Size = Vector2(Particle.Size.X, Particle.Size.Y) *;
			ParticlesInstanceData[i].Size = Vector2(Particle.Size.X * Scale.X, Particle.Size.Y * Scale.Z);
			ParticlesInstanceData[i].Rotation = Particle.Rotation;
			ParticlesInstanceData[i].ParticleId = Particle.Flags & EParticleStates::STATE_CounterMask;

			if (bHasSubuv)
			{
				SubuvPayloadData* PayloadData = (SubuvPayloadData*)((uint8*)&Particle + SubuvOffset);
				ParticlesInstanceData[i].SubImageIndex = PayloadData->ImageIndex;
			}
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
		CommandList->SetStreamSource(1, ParticlesInstanceBuffer, 0);
		CommandList->SetStreamSource(2, DynamicBuffer, 0);
	}

}  // namespace Drn