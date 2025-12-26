#include "DrnPCH.h"
#include "SpotLightSceneProxy.h"
#include "Runtime/Renderer/RenderTexture.h"

#define SPOTLIGHT_SHADOW_SIZE 512
#define SPOTLIGHT_NEAR_Z 0.1f

namespace Drn
{
	SpotLightSceneProxy::SpotLightSceneProxy( class SpotLightComponent* InComponent )
		: LightSceneProxy( InComponent )
		, m_SpotLightComponent(InComponent)
		, m_ShadowmapResource(nullptr)
	{}

	SpotLightSceneProxy::~SpotLightSceneProxy()
	{
		ReleaseShadowmap();
	}

	void SpotLightSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT("SpotLight");

		m_SpotLightData.LocalToWorld = m_LocalToWorld;
		m_SpotLightData.WorldPosition = m_WorldPosition;
		m_SpotLightData.Attenuation = m_Attenuation;
		m_SpotLightData.Direction = m_Direction;
		m_SpotLightData.InvRadius = 1 / m_Attenuation;
		m_SpotLightData.Color = m_LightColor;
		m_SpotLightData.OutterRadius = m_OuterRadius;
		m_SpotLightData.InnerRadius = m_InnerRadius;
		m_SpotLightData.CosOuterCone = std::cos(m_OuterRadius);
		m_SpotLightData.InvCosConeDifference = 1.0f / (1 - (std::cos(m_OuterRadius - m_InnerRadius)));
		m_SpotLightData.ShadowBufferIndex = m_CastShadow ? ShadowDepthBuffer->GetViewIndex() : 0;

		TRefCountPtr<RenderUniformBuffer> LightBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(SpotLightData), EUniformBufferUsage::SingleFrame, &m_SpotLightData);

		CommandList->SetGraphicRootConstant(LightBuffer->GetViewIndex(), 1);

		// TODO: make light flags enum. e. g. 1: Point light. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 2;
		CommandList->SetGraphicRootConstant(LightFlags, 7);

		if (m_CastShadow)
		{
			CommandList->TransitionResourceWithTracking(m_ShadowmapResource->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			CommandList->FlushBarriers();
		}

		CommonResources::Get()->m_SpotLightCone->BindAndDraw(CommandList);
	}

	void SpotLightSceneProxy::RenderShadowDepth( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (m_CastShadow)
		{
			SCOPE_STAT("SpotLight");

			CommandList->SetViewport( 0, 0, 0, SPOTLIGHT_SHADOW_SIZE, SPOTLIGHT_SHADOW_SIZE, 1 );

			CommandList->TransitionResourceWithTracking(m_ShadowmapResource->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
			CommandList->FlushBarriers();

			CommandList->ClearDepthTexture(m_ShadowmapResource, EDepthStencilViewType::DepthWrite, true, false);

			D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_ShadowmapResource->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
			CommandList->GetD3D12CommandList()->OMSetRenderTargets(0, nullptr, false, &Handle);

// ----------------------------------------------------------------------------------------

			XMVECTOR LightPosition = XMLoadFloat3(m_WorldPosition.Get());
			XMVECTOR ViewDirection = XMLoadFloat3(m_Direction.Get());
			XMVECTOR FocusPoint = LightPosition + ViewDirection;

			Matrix ViewMatrix = XMMatrixLookAtLH( LightPosition, FocusPoint, XMLoadFloat3(Vector::UpVector.Get()));
			Matrix ProjectionMatrix = XMMatrixPerspectiveFovLH(m_OuterRadius * 2, 1.0f, SPOTLIGHT_NEAR_Z, m_Attenuation);

			Matrix ViewProjection = ViewMatrix * ProjectionMatrix;
			m_ShadowDepthData.WorldToProjectionMatrices = ViewProjection;

// ----------------------------------------------------------------------------------------

			m_ShadowDepthData.DepthBias = m_DepthBias;
			m_ShadowDepthData.InvShadowResolution = 1.0f / SPOTLIGHT_SHADOW_SIZE;
			m_ShadowDepthData.ShadowmapTextureIndex = m_ShadowmapResource->GetShaderResourceView()->GetDescriptorHeapIndex();

			ShadowDepthBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(SpotLightShadowData), EUniformBufferUsage::SingleFrame, &m_ShadowDepthData);

			//CommandList->SetGraphicRootConstant(ShadowDepthBuffer->GetViewIndex(), 6);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, Renderer, this, ShadowDepthBuffer->GetViewIndex());
			}

		}
	}

	void SpotLightSceneProxy::AllocateShadowmap( D3D12CommandList* CommandList )
	{
		RenderResourceCreateInfo ShadowmapCreateInfo( nullptr, nullptr, ClearValueBinding::DepthOne, "SpotLightShadowmap" );
		m_ShadowmapResource = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), SPOTLIGHT_SHADOW_SIZE, SPOTLIGHT_SHADOW_SIZE, DXGI_FORMAT_D16_UNORM, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), ShadowmapCreateInfo);
	}

	void SpotLightSceneProxy::ReleaseShadowmap()
	{
		m_ShadowmapResource = nullptr;
		ShadowDepthBuffer = nullptr;
	}

	void SpotLightSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		if (m_SpotLightComponent && m_SpotLightComponent->IsRenderStateDirty())
		{
			m_SpotLightComponent->ClearRenderStateDirty();

			m_LightColor = m_SpotLightComponent->GetScaledColor();
			m_CastShadow = m_SpotLightComponent->IsCastingShadow();

			m_LocalToWorld = Transform(m_SpotLightComponent->GetWorldLocation(), m_SpotLightComponent->GetWorldRotation());
			m_WorldPosition = m_SpotLightComponent->GetWorldLocation();
			m_Direction = m_SpotLightComponent->GetWorldRotation().GetVector();

			m_Attenuation = m_SpotLightComponent->GetAttenuation();
			m_InnerRadius = Math::DegreesToRadians(m_SpotLightComponent->GetInnerRadius());
			m_OuterRadius = Math::DegreesToRadians(m_SpotLightComponent->GetOutterRadius());
			m_DepthBias = m_SpotLightComponent->GetDepthBias();
		}

		if (m_CastShadow && !m_ShadowmapResource)
		{
			AllocateShadowmap(CommandList);
		}

		else if (!m_CastShadow && m_ShadowmapResource)
		{
			ReleaseShadowmap();
		}
	}

}