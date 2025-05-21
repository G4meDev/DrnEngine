#include "DrnPCH.h"
#include "PointLightComponent.h"
#include "Runtime/Engine/PointLightSceneProxy.h"

namespace Drn
{
	PointLightComponent::PointLightComponent()
		: LightComponent()
		, m_Radius(3.0f)
	{
		
	}

	PointLightComponent::~PointLightComponent()
	{
		
	}

	void PointLightComponent::RegisterComponent( World* InOwningWorld )
	{
		LightComponent::RegisterComponent(InOwningWorld);

		m_SceneProxy = new PointLightSceneProxy(this);
		InOwningWorld->GetScene()->RegisterLightProxy(m_SceneProxy);
	}

	void PointLightComponent::UnRegisterComponent()
	{
		if (GetWorld())
		{
			GetWorld()->GetScene()->UnRegisterLightProxy( m_SceneProxy );
		}

		LightComponent::UnRegisterComponent();
	}

}