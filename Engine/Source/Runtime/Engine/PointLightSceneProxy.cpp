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

#if WITH_EDITOR
	void PointLightSceneProxy::DrawAttenuation(World* InWorld)
	{
		InWorld->DrawDebugSphere( m_WorldPosition, Quat::Identity, Vector::OneVector, m_Radius, 12, 5, 0 );
	}
#endif
}