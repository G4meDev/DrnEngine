#include "DrnPCH.h"
#include "PointLightComponent.h"

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


	}

	void PointLightComponent::UnRegisterComponent()
	{
		LightComponent::UnRegisterComponent();


	}

}