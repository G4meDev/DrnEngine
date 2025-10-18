#include "DrnPCH.h"
#include "DecalComponent.h"

namespace Drn
{
	DecalComponent::DecalComponent()
		: SceneComponent()
		//, m_SceneProxy(nullptr)
		, m_RenderTransformDirty(true)
		, m_RenderStateDirty(true)
	{
	}

	DecalComponent::~DecalComponent()
	{
	}

	void DecalComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);
		
		if (Ar.IsLoading())
		{
			
		}

		else
		{

		}
	}

	void DecalComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

		//m_SceneProxy = new PostProcessSceneProxy(this);
		//GetWorld()->GetScene()->RegisterPostProcessProxy(m_SceneProxy);

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\ComponentIcons\\T_DecalIcon.drn" );
		DefaultIcon.Load();
		
		m_Sprite->SetSprite( DefaultIcon );
#endif
	}

	void DecalComponent::UnRegisterComponent()
	{
		//GetWorld()->GetScene()->UnRegisterPostProcessProxy(m_SceneProxy);
		//delete m_SceneProxy;
		//m_SceneProxy = nullptr;

		SceneComponent::UnRegisterComponent();
	}

	void DecalComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);

		m_RenderTransformDirty = true;
	}

#if WITH_EDITOR
	void DecalComponent::DrawDetailPanel( float DeltaTime )
	{
		//bool DirtySettings = m_PostProcessSettings.Draw();
		//m_RenderSettingsDirty |= DirtySettings;
	}

	void DecalComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		SceneComponent::SetSelectedInEditor(SelectedInEditor);
	}
#endif

}