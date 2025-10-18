#include "DrnPCH.h"
#include "DecalComponent.h"
#include "Runtime/Engine/DecalSceneProxy.h"

namespace Drn
{
	DecalComponent::DecalComponent()
		: SceneComponent()
		, m_SceneProxy(nullptr)
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

		m_SceneProxy = new DecalSceneProxy(this);
		GetWorld()->GetScene()->RegisterDecalProxy(m_SceneProxy);

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\ComponentIcons\\T_DecalIcon.drn" );
		DefaultIcon.Load();
		
		m_Sprite->SetSprite( DefaultIcon );
#endif
	}

	void DecalComponent::UnRegisterComponent()
	{
		GetWorld()->GetScene()->UnRegisterDecalProxy(m_SceneProxy);
		m_SceneProxy = nullptr;

		SceneComponent::UnRegisterComponent();
	}

	void DecalComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);

		m_RenderTransformDirty = true;
	}

	void DecalComponent::UpdateRenderStateConditional()
	{
		if (m_SceneProxy)
		{
			if (m_RenderTransformDirty)
			{
				m_SceneProxy->m_WorldTransform = GetWorldTransform();
			}
		
			if (m_RenderStateDirty)
			{
				//m_SceneProxy->m_Settings = m_PostProcessSettings;
			}
		
			m_RenderTransformDirty = false;
			m_RenderStateDirty = false;
		}
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

	void DecalComponent::DrawEditorSelected()
	{
		if (GetWorld())
		{
			GetWorld()->DrawDebugBox(Box(Vector(-1.0f), Vector(1.0f)), GetWorldTransform(), Color::White, 0.0f, 0.0f);
		}
	}

#endif

}