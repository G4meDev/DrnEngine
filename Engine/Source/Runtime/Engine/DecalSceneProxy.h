#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	struct DecalData
	{
	public:
		DecalData() = default;

		Matrix DecalToProjection;
		Matrix ProjectionToDecal;
	};

	class DecalSceneProxy
	{
	public:
		DecalSceneProxy( class DecalComponent* InComponent );
		virtual ~DecalSceneProxy();

		inline void Release() { delete this; }
		void ReleaseBuffers();

		void UpdateResources();
		void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer );

		Transform m_WorldTransform;

		class DecalComponent* m_DecalComponent;

		// TODO: make num_buffer
		DecalData m_DecalData;
		Resource* m_DecalBuffer;
	};
}