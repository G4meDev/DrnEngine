#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	struct DecalData
	{
	public:
		DecalData() = default;

		Matrix LocalToProjection;
		Matrix ProjectionToLocal;
	};

	class DecalSceneProxy
	{
	public:
		DecalSceneProxy( class DecalComponent* InComponent );
		virtual ~DecalSceneProxy();

		inline void Release() { delete this; }
		void ReleaseBuffers();

		void UpdateResources( ID3D12GraphicsCommandList2* CommandList );
		void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer );

		Transform m_WorldTransform;
		AssetHandle<Material> m_Material;

		class DecalComponent* m_DecalComponent;

		DecalData m_DecalData;
		Resource* m_DecalBuffer[NUM_BACKBUFFERS] = {nullptr};
		ConstantBufferView m_DecalBufferView[NUM_BACKBUFFERS];
	};
}