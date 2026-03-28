#pragma once

#include "ForwardTypes.h"

class ID3D12GraphicsCommandList2;

namespace Drn
{
	class PrimitiveSceneProxy
	{
	public:
		PrimitiveSceneProxy( const PrimitiveComponent* InComponent);
		virtual ~PrimitiveSceneProxy();

#if WITH_EDITOR
		inline bool IsSelectedInEditor() const { return m_SelectedInEditor; }
		void SetSelectedInEditor( bool Selected ) { m_SelectedInEditor = Selected; }

		inline bool IsSelectable() const { return m_Selectable; }
		inline void SetSelectable( bool Selectable )
		{
			m_Selectable = Selectable;
		}

		inline bool IsEditorPrimitive() const { return m_EditorPrimitive; }
#endif

		inline void SetLocalToWorld( const Matrix& InMatrix ) { m_LocalToWorld = InMatrix; }

		virtual const BoxSphereBounds& GetBounds() = 0;

		inline virtual void SetStatic(bool bInStatic) { bStatic = bInStatic; }
		inline bool IsStatic() const { return bStatic; }

		inline bool IsMarkedPendingKill() const { return bPendingDestory; }
		inline void MarkPendingKill() { bPendingDestory = true; }

	protected:

		virtual void RenderVelocityPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderMainPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderPrePass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderShadowPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy) = 0;
		virtual void RenderDecalPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) = 0;

		virtual void InitResources(class D3D12CommandList* CommandList) = 0;
		virtual void UpdateResources(class D3D12CommandList* CommandList) = 0;
		virtual PrimitiveComponent*  GetPrimitive() = 0;

		Matrix m_LocalToWorld;

		float MinDrawDistance;
		float MaxDrawDistance;

		BoxSphereBounds Bounds;
		BoxSphereBounds LocalBounds;
		Vector ActorPosition;

		bool bStatic;
		bool bPendingDestory;

#if WITH_EDITOR
		virtual void RenderHitProxyPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderSelectionPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderEditorPrimitivePass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) = 0;
		bool m_SelectedInEditor = false;
		bool m_Selectable = false;

		bool m_EditorPrimitive;
#endif

#if D3D12_Debug_INFO
		std::string m_Name;
#endif

	private:

		friend class Scene;
		friend class Renderer;
		friend class SceneRenderer;
		friend class PointLightSceneProxy;
		friend class SpotLightSceneProxy;
		friend class DirectionalLightSceneProxy;
	};
}