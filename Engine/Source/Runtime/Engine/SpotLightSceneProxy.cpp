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

		InWorld->DrawDebugCone(m_LightComponent->GetWorldLocation(), Vector::DownVector, 5, Math::DegreesToRadians(45.0f), Math::DegreesToRadians(45.0f), Color::White, 12, 0, 0);
	}

}