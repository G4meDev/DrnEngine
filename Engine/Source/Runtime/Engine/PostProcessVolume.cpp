#include "DrnPCH.h"
#include "PostProcessVolume.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
	REGISTER_SERIALIZABLE_ACTOR( EActorType::PostProcessVolume, PostProcessVolume );
	DECLARE_LEVEL_SPAWNABLE_CLASS( PostProcessVolume, Volume );

	PostProcessVolumeComponent::PostProcessVolumeComponent()
		: SceneComponent()
		, m_SceneProxy(nullptr)
		, m_RenderTransformDirty(true)
		, m_RenderSettingsDirty(true)
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

		m_SceneProxy = new PostProcessSceneProxy(this);
		GetWorld()->GetScene()->RegisterPostProcessProxy(m_SceneProxy);

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\T_DefaultComponentIcon.drn" );
		DefaultIcon.Load();
		
		m_Sprite->SetSprite( DefaultIcon );
#endif
	}

	void PostProcessVolumeComponent::UnRegisterComponent()
	{
		GetWorld()->GetScene()->UnRegisterPostProcessProxy(m_SceneProxy);
		delete m_SceneProxy;
		m_SceneProxy = nullptr;

		SceneComponent::UnRegisterComponent();
	}

	void PostProcessVolumeComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);

		m_RenderTransformDirty = true;
	}

#if WITH_EDITOR
	void PostProcessVolumeComponent::DrawDetailPanel( float DeltaTime )
	{
		bool DirtySettings = m_PostProcessSettings.Draw();
		m_RenderSettingsDirty |= DirtySettings;
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


	void PostProcessSceneProxy::UpdateResources()
	{
		if (m_PostProcessComponent)
			m_PostProcessComponent->UpdateRenderStateConditional();
	}

}