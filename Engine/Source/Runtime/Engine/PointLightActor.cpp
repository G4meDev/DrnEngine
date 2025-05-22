#include "DrnPCH.h"
#include "PointLightActor.h"
#include "Runtime/Components/PointLightComponent.h"
#include "Runtime/Components/BillboardComponent.h"

namespace Drn
{
	PointLightActor::PointLightActor()
		: LightActor()
	{
		m_PointLightComponent = std::make_unique<class PointLightComponent>();
		m_PointLightComponent->SetComponentLabel( "LightComponent" );
		SetRootComponent(m_PointLightComponent.get());
		SetLightComponent(m_PointLightComponent.get());

	}

	PointLightActor::~PointLightActor()
	{
		
	}

}