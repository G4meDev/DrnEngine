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
		//drn_check(OwningEmitter->HasLightModule);

		if (MaxParticles < OwningEmitter->MaxActiveParticles)
		{
			ParticlesLightInstanceData.resize(OwningEmitter->MaxActiveParticles);
		}

		ActiveParticles = OwningEmitter->ActiveParticles;
		MaxParticles = OwningEmitter->MaxActiveParticles;

		for (int32 i = 0; i < ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, OwningEmitter->ParticleData + OwningEmitter->ParticleStride * OwningEmitter->ParticleIndices[i]);

			Vector Color = 1;
			float Radius = 1;

			ParticlesLightInstanceData[i].WorldPosition = OwningEmitter->SimulationToWorld.TransformPosition(Particle.Location);
			ParticlesLightInstanceData[i].Color = Color;
			ParticlesLightInstanceData[i].Radius = Radius;
			ParticlesLightInstanceData[i].InvRadius = 1.0f/Radius;
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