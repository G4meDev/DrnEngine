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

		inline bool IsMarkedPendingKill() const { return bPendingDestory; }
		inline void MarkPendingKill() { bPendingDestory = true; }

		void UpdateResources( class D3D12CommandList* CommandList );
		void Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer );

		BoxSphereBounds GetBounds();

		Transform m_WorldTransform;
		MaterialSlot m_Material;

		class DecalComponent* m_DecalComponent;

		DecalData m_DecalData;

	protected:

		BoxSphereBounds Bounds;
		bool bPendingDestory;
	};
}