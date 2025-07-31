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
#endif

		inline void SetLocalToWorld( const Matrix& InMatrix ) { m_LocalToWorld = InMatrix; }

	protected:

		virtual void RenderMainPass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderShadowPass(ID3D12GraphicsCommandList2* CommandList, LightSceneProxy* LightProxy) = 0;

		virtual void InitResources(ID3D12GraphicsCommandList2* CommandList) = 0;
		virtual void UpdateResources(ID3D12GraphicsCommandList2* CommandList) = 0;
		virtual PrimitiveComponent*  GetPrimitive() = 0;

		bool m_EditorPrimitive;
		Matrix m_LocalToWorld;

#if WITH_EDITOR
		virtual void RenderHitProxyPass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderSelectionPass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderEditorPrimitivePass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer) = 0;
		bool m_SelectedInEditor = false;
		bool m_Selectable = false;
#endif

#if D3D12_Debug_INFO
		std::string m_Name;
#endif

	private:

		friend class Scene;
		friend class Renderer;
		friend class SceneRenderer;
		friend class PointLightSceneProxy;
	};
}