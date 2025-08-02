#include "DrnPCH.h"
#include "SpotLightSceneProxy.h"

namespace Drn
{
	SpotLightSceneProxy::SpotLightSceneProxy( class SpotLightComponent* InComponent )
		: LightSceneProxy( InComponent )
	{
		
	}

	SpotLightSceneProxy::~SpotLightSceneProxy()
	{
		
	}

	void SpotLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void SpotLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void SpotLightSceneProxy::DrawAttenuation( World* InWorld )
	{
		
	}

}