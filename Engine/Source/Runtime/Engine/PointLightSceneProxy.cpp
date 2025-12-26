#include "DrnPCH.h"
#include "PointLightSceneProxy.h"
#include "Runtime/Components/PointLightComponent.h"
#include "Runtime/Renderer/RenderTexture.h"

#define POINTLIGHT_SHADOW_SIZE 512
#define POINTLIGHT_NEAR_Z 0.1f

namespace Drn
{
	PointLightSceneProxy::PointLightSceneProxy( class PointLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_PointLightComponent(InComponent)
		, m_ShadowCubemapResource(nullptr)
	{}

	PointLightSceneProxy::~PointLightSceneProxy()
	{
		ReleaseShadowmap();
		ReleaseBuffer();
	}

	void PointLightSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT("PointLight");

		m_Buffer.WorldPosition = m_WorldPosition;
		m_Buffer.Scale = m_Radius;
		m_Buffer.Color = m_LightColor;
		m_Buffer.InvRadius = 1 / m_Radius;
		m_Buffer.ShadowBufferIndex = m_CastShadow ? ShadowDepthBuffer->GetViewIndex() : 0;

		TRefCountPtr<RenderUniformBuffer> LightBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PointLightBuffer), EUniformBufferUsage::SingleFrame, &m_Buffer);

		CommandList->SetGraphicRootConstant(LightBuffer->GetViewIndex(), 1);

		// TODO: make light flags enum. e. g. 1: Point light. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 1;
		CommandList->SetGraphicRootConstant(LightFlags, 7);

		if (m_CastShadow)
		{
			CommandList->TransitionResourceWithTracking(m_ShadowCubemapResource->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			CommandList->FlushBarriers();
		}

		CommonResources::Get()->m_PointLightSphere->BindAndDraw(CommandList);
	}

	void PointLightSceneProxy::RenderShadowDepth( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (m_CastShadow)
		{
			SCOPE_STAT("PointLight");

			CommandList->SetViewport(0 ,0, 0, POINTLIGHT_SHADOW_SIZE, POINTLIGHT_SHADOW_SIZE, 1);

			CommandList->TransitionResourceWithTracking(m_ShadowCubemapResource->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
			CommandList->FlushBarriers();

			CommandList->ClearDepthTexture(m_ShadowCubemapResource, EDepthStencilViewType::DepthWrite, true, false);
			D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_ShadowCubemapResource->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
			CommandList->GetD3D12CommandList()->OMSetRenderTargets(0, nullptr, false, &Handle);

			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[0], Vector::RightVector, Vector::UpVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[1], Vector::LeftVector, Vector::UpVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[2], Vector::UpVector, Vector::BackwardVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[3], Vector::DownVector, Vector::ForwardVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[4], Vector::ForwardVector, Vector::UpVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[5], Vector::BackwardVector, Vector::UpVector);

			m_ShadowDepthData.DepthBias = m_DepthBias;
			m_ShadowDepthData.InvShadowResolution = 1.0f / POINTLIGHT_SHADOW_SIZE;
			m_ShadowDepthData.ShadowmapTextureIndex = m_ShadowCubemapResource->GetShaderResourceView()->GetDescriptorHeapIndex();

			ShadowDepthBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(ShadowDepthData), EUniformBufferUsage::SingleFrame, &m_ShadowDepthData);

			//CommandList->SetGraphicRootConstant(ShadowDepthBuffer->GetViewIndex(), 6);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, Renderer, this, ShadowDepthBuffer->GetViewIndex());
			}

		}
	}

	void PointLightSceneProxy::AllocateShadowmap( D3D12CommandList* CommandList )
	{
		RenderResourceCreateInfo ShadowmapCubemapCreateInfo( nullptr, nullptr, ClearValueBinding::DepthOne, "PointLightShadowmap" );
		m_ShadowCubemapResource = RenderTextureCube::Create(Renderer::Get()->GetCommandList_Temp(), POINTLIGHT_SHADOW_SIZE, DXGI_FORMAT_D16_UNORM, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), ShadowmapCubemapCreateInfo);
	}

	void PointLightSceneProxy::ReleaseShadowmap()
	{
		m_ShadowCubemapResource = nullptr;
		ShadowDepthBuffer = nullptr;
	}

	void PointLightSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		if (m_PointLightComponent && m_PointLightComponent->IsRenderStateDirty())
		{
			m_PointLightComponent->ClearRenderStateDirty();

			m_CastShadow = m_PointLightComponent->IsCastingShadow();
			m_WorldPosition = m_PointLightComponent->GetWorldLocation();
			m_LightColor = m_PointLightComponent->GetScaledColor();

			m_Radius = m_PointLightComponent->GetRadius();
			m_DepthBias = m_PointLightComponent->GetDepthBias();
		}

		if (m_CastShadow && !m_ShadowCubemapResource)
		{
			AllocateShadowmap(CommandList);
		}

		else if (!m_CastShadow && m_ShadowCubemapResource)
		{
			ReleaseShadowmap();
		}
	}

	void PointLightSceneProxy::ReleaseBuffer()
	{}

	void PointLightSceneProxy::CalculateLocalToProjectionForDirection( Matrix& Mat, const Vector& Direction, const Vector& UpVector)
	{
		XMVECTOR LightPosition = XMLoadFloat3(m_WorldPosition.Get());
		XMVECTOR ViewDirection = XMLoadFloat3(Direction.Get());
		XMVECTOR FocusPoint = LightPosition + ViewDirection;

		Matrix ViewMatrix = XMMatrixLookAtLH( LightPosition, FocusPoint, XMLoadFloat3(UpVector.Get()));
		Matrix ProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, 1.0f, POINTLIGHT_NEAR_Z, m_Radius);

		Matrix ViewProjection = ViewMatrix * ProjectionMatrix;
		
		Mat = ViewProjection;
	}

}