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

		//PostProcessSceneProxy* m_SceneProxy;

		//inline void UpdateRenderStateConditional()
		//{
		//	if (m_SceneProxy)
		//	{
		//		if (m_RenderTransformDirty)
		//		{
		//			m_SceneProxy->m_WorldTransform = GetWorldTransform();
		//		}
		//
		//		if (m_RenderSettingsDirty)
		//		{
		//			m_SceneProxy->m_Settings = m_PostProcessSettings;
		//		}
		//
		//		m_RenderTransformDirty = false;
		//		m_RenderSettingsDirty = false;
		//	}
		//}

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		inline virtual bool HasSprite() const override { return true; }
		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
#endif
	};
}