#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/SceneComponent.h"
#include "Runtime/Engine/PostProcessSettings.h"

namespace Drn
{
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

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
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