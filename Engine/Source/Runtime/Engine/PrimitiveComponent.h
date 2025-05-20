#pragma once

#include "ForwardTypes.h"
#include "SceneComponent.h"

#include "Runtime/Physic/BodyInstance.h"

namespace Drn
{
	class PrimitiveComponent : public SceneComponent
	{
	public:
		PrimitiveComponent();
		virtual ~PrimitiveComponent();

		virtual void Serialize( Archive& Ar ) override;

		virtual void Tick(float DeltaTime) override;

		inline BodyInstance GetBodyInstance() { return m_BodyInstance; }

		void SendPhysicsTransform();

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }
		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		//virtual PrimitiveSceneProxy* AllocateSceneProxy() = 0;
		//virtual PrimitiveSceneProxy* GetSceneProxy() const = 0;

		inline bool IsEditorPrimitive() const { return m_EditorPrimitive; }
		inline void SetEditorPrimitive(bool EditorPrimitive) { m_EditorPrimitive = EditorPrimitive; }

		inline bool IsSelectable() const { return m_Selectable; }
		virtual void SetSelectable( bool Selectable );

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
#endif

	protected:

		bool m_RenderStateDirty;
		bool m_EditorPrimitive;

#if WITH_EDITOR
		bool m_Selectable = true;
#endif

		BodyInstance m_BodyInstance;

	private:
	};

}