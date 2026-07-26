#include "DrnPCH.h"
#include "ParticleLightSceneProxy.h"

namespace Drn
{
	ParticleLightSceneProxy::ParticleLightSceneProxy( ParticleEmitterInstance* InOwningEmitter )
		: OwningEmitter(InOwningEmitter)
		, ActiveParticles(0)
		, MaxParticles(0)
	{
		
	}

	void ParticleLightSceneProxy::Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (ActiveParticles > 0)
		{
			SCOPE_STAT("ParticleLight");
			drn_check(ParticlesLightInstanceBuffer);

			{
				SCOPE_STAT("Bounds");

				Sphere ParticleBounds;
				ParticleBounds.Init();
				for (int32 ParticleIndex = 0; ParticleIndex < ActiveParticles; ParticleIndex++)
				{
					const ParticleLightInstance& Instance = ParticlesLightInstanceData[ParticleIndex];
					ParticleBounds += Sphere(Instance.WorldPosition, Instance.Radius);
				}

				if (!ParticleBounds.IsValid() || !Renderer->GetViewFrustum().Contains(ParticleBounds))
					return;
			}

			CommandList->SetGraphicPipelineState(CommonResources::Get()->m_LightPassPSO->m_ParticleLightPass_PSO);

			CommandList->SetStreamSource(0, CommonResources::Get()->m_PointLightSphere->m_VertexBuffer, 0);
			CommandList->SetStreamSource(1, ParticlesLightInstanceBuffer, 0);
			CommandList->DrawIndexedPrimitive(CommonResources::Get()->m_PointLightSphere->m_IndexBuffer, 0, 0,
				CommonResources::Get()->m_PointLightSphere->VertexCount, 0, CommonResources::Get()->m_PointLightSphere->PrimitiveCount, ActiveParticles);
		}
	}

	void ParticleLightSceneProxy::UpdateResources( class D3D12CommandList* CommandList )
	{
		drn_check(OwningEmitter);
		drn_check(OwningEmitter->Emitter->IsLightActive());

		if (MaxParticles < OwningEmitter->MaxActiveParticles)
		{
			ParticlesLightInstanceData.resize(OwningEmitter->MaxActiveParticles);
		}

		ActiveParticles = OwningEmitter->ActiveParticles;
		MaxParticles = OwningEmitter->MaxActiveParticles;

		const int32 LightOffset = OwningEmitter->Emitter->GetLightOffset();
		drn_check(LightOffset);

		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * OwningEmitter->ParticleIndices[i]);

			ParticleLightPayload* PayloadData = (ParticleLightPayload*)( (uint8*)&Particle + LightOffset );

			ParticlesLightInstanceData[i].WorldPosition = OwningEmitter->SimulationToWorld.TransformPosition(Particle.Location);
			ParticlesLightInstanceData[i].Color = PayloadData->Color;
			ParticlesLightInstanceData[i].Radius = PayloadData->Radius;
			ParticlesLightInstanceData[i].InvRadius = 1.0f/PayloadData->Radius;
		}

		if (ActiveParticles > 0)
		{
			drn_check(MaxParticles > 0);
			drn_check(MaxParticles >= ActiveParticles);

			{
				uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Dynamic;
				RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, ParticlesLightInstanceData.data(), ClearValueBinding::Black, "ParticlesLightInstanceData");
				ParticlesLightInstanceBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, MaxParticles * sizeof(ParticleLightInstance), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
			}
		}
		else
		{
			ParticlesLightInstanceBuffer = nullptr;
		}
		
	}

}  // namespace Drn