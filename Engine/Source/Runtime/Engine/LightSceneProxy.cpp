#include "DrnPCH.h"
#include "LightSceneProxy.h"
#include "Runtime/Components/LightComponent.h"

namespace Drn
{
	LightSceneProxy::LightSceneProxy( class LightComponent* InComponent )
		: m_LightComponent( InComponent )
#if D3D12_Debug_INFO
		, m_Name( InComponent ? InComponent->GetComponentLabel() : "InvalidLightComponent" )
#endif
	{
	}

	LightSceneProxy::~LightSceneProxy()
	{
	}

}