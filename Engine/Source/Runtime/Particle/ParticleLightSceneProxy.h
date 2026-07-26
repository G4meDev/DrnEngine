#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitterInstance;

	struct ParticleLightInstance
	{
		Vector WorldPosition;
		float Radius;
		Vector Color;
		float InvRadius;
	};

	class ParticleLightSceneProxy
	{
	public:
		ParticleLightSceneProxy(ParticleEmitterInstance* InOwningEmitter);

		void UpdateResources( class D3D12CommandList* CommandList );
		void Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer );

		ParticleEmitterInstance* OwningEmitter;
		int32 ActiveParticles;
		int32 MaxParticles;

		std::vector<ParticleLightInstance> ParticlesLightInstanceData;
		TRefCountPtr<RenderVertexBuffer> ParticlesLightInstanceBuffer;
	};
}