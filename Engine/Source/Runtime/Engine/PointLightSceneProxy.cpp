#include "DrnPCH.h"
#include "PointLightSceneProxy.h"
#include "Runtime/Components/PointLightComponent.h"

namespace Drn
{
	PointLightSceneProxy::PointLightSceneProxy( class PointLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_Radius(InComponent->GetRadius())
	{
	}

	PointLightSceneProxy::~PointLightSceneProxy()
	{
	}

}