#include "DrnPCH.h"
#include "SpotLightSceneProxy.h"

namespace Drn
{
	SpotLightSceneProxy::SpotLightSceneProxy( class SpotLightComponent* InComponent )
		: LightSceneProxy( InComponent )
	{
		m_SpotLightComponent = InComponent;

		SetDirection(InComponent->GetWorldRotation().GetVector());
		SetAttenuation(InComponent->GetAttenuation());
		SetOutterRadius(InComponent->GetOutterRadius());
		SetInnerRadius(InComponent->GetInnerRadius());

		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ);

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());

#if D3D12_Debug_INFO
		m_LightBuffer->SetName("CB_SpotLight_" + m_Name);
#endif
	}

	SpotLightSceneProxy::~SpotLightSceneProxy()
	{
		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}
	}

	void SpotLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		// TODO: remove. should not be aware of component. lazy update
		m_SpotLightData.LocalToWorld =  Transform(m_SpotLightComponent->GetWorldLocation(), m_SpotLightComponent->GetWorldRotation());
		m_SpotLightData.WorldPosition = m_SpotLightComponent->GetWorldLocation();
		m_SpotLightData.Attenuation = m_SpotLightComponent->GetAttenuation();
		m_SpotLightData.Direction = m_SpotLightComponent->GetWorldRotation().GetVector();
		m_SpotLightData.InvRadius = 1 / m_SpotLightData.Attenuation;
		m_SpotLightData.Color = m_SpotLightComponent->GetScaledColor();
		m_SpotLightData.OutterRadius = Math::DegreesToRadians(m_SpotLightComponent->GetOutterRadius());
		m_SpotLightData.InnerRadius = Math::DegreesToRadians(m_SpotLightComponent->GetInnerRadius());
		m_SpotLightData.CosOuterCone = std::cos(m_SpotLightData.OutterRadius);
		m_SpotLightData.InvCosConeDifference = 1.0f / (1 - (std::cos(m_SpotLightData.OutterRadius - m_SpotLightData.InnerRadius)));
		
		//m_SpotLightData.ShadowBufferIndex = m_CastShadow ? Renderer::Get()->GetBindlessSrvIndex(m_ShadowDepthBuffer->GetGpuHandle()) : 0;
		m_SpotLightData.ShadowBufferIndex = 0;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_LightBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_SpotLightData, sizeof(SpotLightData));
		m_LightBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

		// TODO: make light flags enum. e. g. 1: Pointlight. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 2;
		CommandList->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

		CommonResources::Get()->m_SpotLightCone->BindAndDraw(CommandList);
	}

	void SpotLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void SpotLightSceneProxy::DrawAttenuation( World* InWorld )
	{
		InWorld->DrawDebugCone(m_SpotLightData.WorldPosition, m_SpotLightData.Direction, m_SpotLightData.Attenuation,
			m_SpotLightData.OutterRadius, m_SpotLightData.OutterRadius, Color::White, 32, 0, 0);

		InWorld->DrawDebugConeCap(m_SpotLightData.WorldPosition, m_SpotLightData.Direction, m_SpotLightData.Attenuation,
			m_SpotLightData.OutterRadius, Color::White, 16, 0, 0);

		if (m_SpotLightData.InnerRadius > 0)
		{
			InWorld->DrawDebugCone(m_SpotLightData.WorldPosition, m_SpotLightData.Direction, m_SpotLightData.Attenuation,
				m_SpotLightData.InnerRadius, m_SpotLightData.InnerRadius, Color::Blue, 32, 0, 0);
		}
	}

}