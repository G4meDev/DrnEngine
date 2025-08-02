#include "DrnPCH.h"
#include "SpotLightComponent.h"

#include "Runtime/Engine/SpotLightSceneProxy.h"

namespace Drn
{
	SpotLightComponent::SpotLightComponent() : LightComponent()
	{

	}

	SpotLightComponent::~SpotLightComponent()
	{
	
	}

 void SpotLightComponent::RegisterComponent( World* InOwningWorld )
	{
		LightComponent::RegisterComponent(InOwningWorld);

		m_SpotLightSceneProxy = new SpotLightSceneProxy(this);
		InOwningWorld->GetScene()->RegisterLightProxy(m_SpotLightSceneProxy);
		m_LightSceneProxy = m_SpotLightSceneProxy;

#if WITH_EDITOR
		AssetHandle<Texture2D> PointLightIcon( "Engine\\Content\\EditorResources\\LightIcons\\T_SpotLightIcon.drn" );
		PointLightIcon.Load();

		m_Sprite->SetSprite( PointLightIcon );
#endif
	}

	void SpotLightComponent::UnRegisterComponent()
	{
		if (GetWorld())
		{
			GetWorld()->GetScene()->UnRegisterLightProxy( m_SpotLightSceneProxy);
		}

		m_SpotLightSceneProxy = nullptr;
		m_LightSceneProxy = nullptr;

		LightComponent::UnRegisterComponent();
	}

}