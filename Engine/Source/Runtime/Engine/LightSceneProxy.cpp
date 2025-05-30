#include "DrnPCH.h"
#include "LightSceneProxy.h"
#include "Runtime/Components/LightComponent.h"

namespace Drn
{
	LightSceneProxy::LightSceneProxy( class LightComponent* InComponent )
		: m_LightComponent( InComponent )
		, m_WorldPosition( InComponent->GetWorldLocation() )
		, m_LightColor( InComponent->GetScaledColor() )
		, m_CastShadow( InComponent->IsCastingShadow() )
	{
#if WITH_EDITOR
		m_SelectedInEditor = InComponent->IsSelectedInEditor();
#endif
	}

	LightSceneProxy::~LightSceneProxy()
	{
	}

}