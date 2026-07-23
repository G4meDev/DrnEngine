#include "DrnPCH.h"
#include "ParticleMeshSceneProxy.h"
#include "Runtime/Particle/ParticleEmitterType.h"

namespace Drn
{
	ParticleMeshSceneProxy::ParticleMeshSceneProxy( ParticleMeshEmitterInstance* InOwningEmitter )
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

		TempBound = BoxSphereBounds( Vector::ZeroVector, Vector(500), 1000 );
	}

	ParticleMeshSceneProxy::~ParticleMeshSceneProxy()
	{
	}

	const BoxSphereBounds& ParticleMeshSceneProxy::GetBounds()
	{
		//drn_check(OwningEmitter);
		//return OwningEmitter->GetBoundingBox();

		return TempBound;
	}

	void ParticleMeshSceneProxy::RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleMeshSceneProxy::RenderTranslucencyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleMeshSceneProxy::RenderDistortionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleMeshSceneProxy::RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleMeshSceneProxy::RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleMeshSceneProxy::RenderShadowPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
	{
		
	}

	void ParticleMeshSceneProxy::RenderDecalPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

#if WITH_EDITOR
	void ParticleMeshSceneProxy::RenderHitProxyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleMeshSceneProxy::RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void ParticleMeshSceneProxy::RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}
#endif

	void ParticleMeshSceneProxy::InitResources( class D3D12CommandList* CommandList )
	{
		UpdateMeshAndMaterials(CommandList);
	}

	void ParticleMeshSceneProxy::UpdateResources( class D3D12CommandList* CommandList )
	{
		UpdateBuffers(CommandList);
	}

	void ParticleMeshSceneProxy::UpdateMeshAndMaterials( class D3D12CommandList* CommandList )
	{
		drn_check(OwningEmitter && OwningEmitter->Emitter && OwningEmitter->Emitter->GetEmitterType());
		ParticleEmitterMeshType* EmitterMeshType = OwningEmitter->Emitter->GetEmitterType()->GetType() == EEmitterType::Mesh ? 
			static_cast<ParticleEmitterMeshType*>(OwningEmitter->Emitter->GetEmitterType()) : nullptr;

		Mesh = EmitterMeshType->Mesh;
		Mesh->UploadResources(CommandList);

		Materials.clear();

		if (Mesh.IsValid())
		{
			const uint32 MaterialCount = Mesh->GetMeshData().Materials.size();
			const uint32 OverrideMaterialCount = EmitterMeshType->Materials.size();
			Materials.resize(MaterialCount);

			for (int i = 0; i < MaterialCount; i++)
			{
				if (i < OverrideMaterialCount)
				{
					Materials[i] = EmitterMeshType->Materials[i];
				}
				else
				{
					Materials[i] = Mesh->GetMeshData().Materials[i];
				}

				// TODO: mark this only in with editor builds
				Materials[i].LoadChecked();
				if (!Materials[i].IsValid())
				{
					//LOG(LogStaticMeshSceneProxy, Error, "Material is invalid. Using default material.");

					Materials[i] = AssetHandle<Material>(DEFAULT_MATERIAL_PATH);
					Materials[i].Load();
				}
			}
		}

		for (MaterialSlot& MatSlot : Materials)
		{
			MatSlot.GetMaterialInterface()->UploadResources(CommandList);
		}
	}

	void ParticleMeshSceneProxy::UpdateBuffers( class D3D12CommandList* CommandList )
	{
		drn_check(OwningEmitter);

		if (MaxParticles < OwningEmitter->MaxActiveParticles)
		{
			ParticlesInstanceData.resize(OwningEmitter->MaxActiveParticles);
		}

		ActiveParticles = OwningEmitter->ActiveParticles;
		MaxParticles = OwningEmitter->MaxActiveParticles;

		ParticleData.m_Guid = Guid;
		ParticleBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(ParticleMeshData), EUniformBufferUsage::MultiFrame, &ParticleData);

		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * OwningEmitter->ParticleIndices[i]);
			MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + OwningEmitter->Emitter->GetMeshRotationOffset());

			Quat ParticleRotation = OwningEmitter->Emitter->bHasMeshRotation ? Quat(Math::DegreesToRadians(PayloadData->Rotation.GetX()),
				Math::DegreesToRadians(PayloadData->Rotation.GetY()), Math::DegreesToRadians(PayloadData->Rotation.GetZ())) : Quat::Identity;

			Matrix WorldTransform = OwningEmitter->SimulationToWorld * Transform(Particle.Location, ParticleRotation, Particle.Size);
			ParticlesInstanceData[i].Transform[0] = Vector4(WorldTransform.m_Matrix._11, WorldTransform.m_Matrix._12, WorldTransform.m_Matrix._13, WorldTransform.m_Matrix._14);
			ParticlesInstanceData[i].Transform[1] = Vector4(WorldTransform.m_Matrix._21, WorldTransform.m_Matrix._22, WorldTransform.m_Matrix._23, WorldTransform.m_Matrix._24);
			ParticlesInstanceData[i].Transform[2] = Vector4(WorldTransform.m_Matrix._31, WorldTransform.m_Matrix._32, WorldTransform.m_Matrix._33, WorldTransform.m_Matrix._34);

			ParticlesInstanceData[i].Color = Particle.Color;
			ParticlesInstanceData[i].RelativeTime = Particle.RelativeTime;
			Vector WorldVelocity = OwningEmitter->Emitter->bUseLocalSpace ? OwningEmitter->SimulationToWorld.TransformVector(Particle.Velocity) : Particle.Velocity;
			ParticlesInstanceData[i].Velocity = Vector4(WorldVelocity.GetSafeNormal(), WorldVelocity.Length());
		}

		if (ActiveParticles > 0)
		{
			drn_check(MaxParticles > 0);
			drn_check(MaxParticles >= ActiveParticles);

			{
				uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Dynamic;
				RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, ParticlesInstanceData.data(), ClearValueBinding::Black, "ParticlesInstanceData");
				ParticlesInstanceBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, MaxParticles * sizeof(MeshParticleInstanceVertex), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
			}
		}
		else
		{
			ParticlesInstanceBuffer = nullptr;
			PrevTransformBuffer = nullptr;
			DynamicBuffer = nullptr;
		}
	}

	void ParticleMeshSceneProxy::BindInstanceBuffers( class D3D12CommandList* CommandList )
	{
		CommandList->SetStreamSource(8, ParticlesInstanceBuffer, 0);
		CommandList->SetStreamSource(9, PrevTransformBuffer, 0);
		CommandList->SetStreamSource(10, DynamicBuffer, 0);
	}

}  // namespace Drn