#include "DrnPCH.h"
#include "LightComponent.h"

namespace Drn
{
	LightComponent::LightComponent()
		: m_LightColor(Vector::OneVector)
		, m_Intensity(1.0f)
		, m_CastShadow(false)
	{
	}

	LightComponent::~LightComponent()
	{
	}

	void LightComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);


	}

	void LightComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();

	}

	void LightComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);
	}

}