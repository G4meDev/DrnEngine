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
#endif

	protected:

		virtual void RenderMainPass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer) = 0;
		virtual void RenderSelectionPass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer) = 0;

		virtual void InitResources(ID3D12GraphicsCommandList2* CommandList) = 0;
		virtual void UpdateResources(ID3D12GraphicsCommandList2* CommandList) = 0;
		virtual PrimitiveComponent*  GetPrimitive() = 0;

		bool m_EditorPrimitive;

#if WITH_EDITOR
		bool m_SelectedInEditor = false;
#endif

	private:

		friend class Scene;
		friend class Renderer;
		friend class SceneRenderer;
	};
}