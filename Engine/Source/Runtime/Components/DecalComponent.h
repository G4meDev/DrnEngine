#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SceneComponent.h"
#include "Runtime/Core/Archive.h"

namespace Drn
{
	class DecalComponent : public SceneComponent
	{
	public:
		DecalComponent();
		virtual ~DecalComponent();

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		bool m_RenderTransformDirty;
		bool m_RenderStateDirty;

		class DecalSceneProxy* m_SceneProxy;

		void UpdateRenderStateConditional();

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		inline virtual bool HasSprite() const override { return true; }
		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
		void DrawEditorSelected() override;
#endif
	};
}