#include "DrnPCH.h"
#include "PostProcessVolume.h"

namespace Drn
{
	PostProcessVolumeComponent::PostProcessVolumeComponent()
		: SceneComponent()
	{
	}

	PostProcessVolumeComponent::~PostProcessVolumeComponent()
	{
		
	}

	void PostProcessVolumeComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);
		m_PostProcessSettings.Serialize(Ar);
	}

	void PostProcessVolumeComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);
	}

	void PostProcessVolumeComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();
	}

	void PostProcessVolumeComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);
	}

#if WITH_EDITOR
	void PostProcessVolumeComponent::DrawDetailPanel( float DeltaTime )
	{
		m_PostProcessSettings.Draw();
	}

	void PostProcessVolumeComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		SceneComponent::SetSelectedInEditor(SelectedInEditor);
	}
#endif

// -----------------------------------------------------------------------------------------

	PostProcessVolume::PostProcessVolume()
		: Actor()
	{
		m_PostProcessVolumeComponent = std::make_unique<PostProcessVolumeComponent>();
		SetRootComponent(m_PostProcessVolumeComponent.get());
	}

	PostProcessVolume::~PostProcessVolume()
	{
		
	}

	void PostProcessVolume::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);
	}


}