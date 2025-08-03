#include "DrnPCH.h"
#include "SpotLightSceneProxy.h"

namespace Drn
{
	SpotLightSceneProxy::SpotLightSceneProxy( class SpotLightComponent* InComponent )
		: LightSceneProxy( InComponent )
	{
		SetDirection(InComponent->GetWorldRotation().GetVector());
		SetAttenuation(InComponent->GetAttenuation());
		SetOutterRadius(InComponent->GetOutterRadius());
		SetInnerRadius(InComponent->GetInnerRadius());
	}

	SpotLightSceneProxy::~SpotLightSceneProxy()
	{
		
	}

	void SpotLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		CommonResources::Get()->m_SpotLightCone->BindAndDraw(CommandList);
	}

	void SpotLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void SpotLightSceneProxy::DrawAttenuation( World* InWorld )
	{
		InWorld->DrawDebugCone(m_LightComponent->GetWorldLocation(), m_SpotLightData.Direction, m_SpotLightData.Attenuation,
			Math::DegreesToRadians(m_SpotLightData.OutterRadius), Math::DegreesToRadians(m_SpotLightData.OutterRadius), Color::White, 32, 0, 0);

		InWorld->DrawDebugConeCap(m_LightComponent->GetWorldLocation(), m_SpotLightData.Direction, m_SpotLightData.Attenuation,
			Math::DegreesToRadians(m_SpotLightData.OutterRadius), Color::White, 16, 0, 0);

		if (m_SpotLightData.InnerRadius > 0)
		{
			InWorld->DrawDebugCone(m_LightComponent->GetWorldLocation(), m_SpotLightData.Direction, m_SpotLightData.Attenuation,
				Math::DegreesToRadians(m_SpotLightData.InnerRadius), Math::DegreesToRadians(m_SpotLightData.InnerRadius), Color::Blue, 32, 0, 0);
		}
	}

}