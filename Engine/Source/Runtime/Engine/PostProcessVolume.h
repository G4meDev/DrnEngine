#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/SceneComponent.h"
#include "Runtime/Engine/PostProcessSettings.h"

namespace Drn
{
	// copy that lives on gpu
	class PostProcessSceneProxy
	{
	public:
		PostProcessSceneProxy( class PostProcessVolumeComponent* InComponent )
			: m_PostProcessComponent(InComponent)
		{};
		virtual ~PostProcessSceneProxy() {};

		void UpdateResources();

		// scaled world transform
		Transform m_WorldTransform;
		PostProcessSettings m_Settings;

		class PostProcessVolumeComponent* m_PostProcessComponent;
	};

	class PostProcessVolumeComponent : public SceneComponent
	{
	public:
		PostProcessVolumeComponent();
		virtual ~PostProcessVolumeComponent();

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		PostProcessSettings m_PostProcessSettings;

		bool m_RenderTransformDirty;
		bool m_RenderSettingsDirty;

		PostProcessSceneProxy* m_SceneProxy;

		inline void UpdateRenderStateConditional()
		{
			if (m_SceneProxy)
			{
				if (m_RenderTransformDirty)
				{
					m_SceneProxy->m_WorldTransform = GetWorldTransform();
				}

				if (m_RenderSettingsDirty)
				{
					m_SceneProxy->m_Settings = m_PostProcessSettings;
				}

				m_RenderTransformDirty = false;
				m_RenderSettingsDirty = false;
			}
		}

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		inline virtual bool HasSprite() const override { return true; }
		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
#endif
	};

	class PostProcessVolume : public Actor
	{
	public:
		PostProcessVolume();
		virtual ~PostProcessVolume();

		virtual void Serialize(Archive& Ar) override;

		virtual void Tick(float DeltaTime) override {};
		inline virtual EActorType GetActorType() override { return EActorType::PostProcessVolume; }

		std::unique_ptr<PostProcessVolumeComponent> m_PostProcessVolumeComponent;

	};
}