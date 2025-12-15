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
		, m_LightBuffer(nullptr)
		, m_ShadowDepthBuffer(nullptr)
		, m_ShadowmapResource(nullptr)
	{
		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_LightBuffer->SetName("CB_SpotLight_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());
	}

	SpotLightSceneProxy::~SpotLightSceneProxy()
	{
		ReleaseShadowmap();

		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}
	}

	void SpotLightSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
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
		m_SpotLightData.ShadowBufferIndex = m_CastShadow ? Renderer::Get()->GetBindlessSrvIndex(m_ShadowDepthBuffer->GetGpuHandle()) : 0;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_LightBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_SpotLightData, sizeof(SpotLightData));
		m_LightBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

		// TODO: make light flags enum. e. g. 1: Point light. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 2;
		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

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

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_ShadowDepthBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &m_ShadowDepthData, sizeof(SpotLightData));
			m_ShadowDepthBuffer->GetD3D12Resource()->Unmap(0, nullptr);

			CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
			CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ShadowDepthBuffer->GetGpuHandle()), 6);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, Renderer, this);
			}

		}
	}

	void SpotLightSceneProxy::AllocateShadowmap( D3D12CommandList* CommandList )
	{
		RenderResourceCreateInfo ShadowmapCreateInfo( nullptr, nullptr, ClearValueBinding::DepthOne, "SpotLightShadowmap" );
		m_ShadowmapResource = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), SPOTLIGHT_SHADOW_SIZE, SPOTLIGHT_SHADOW_SIZE, DXGI_FORMAT_D16_UNORM, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), ShadowmapCreateInfo);

// --------------------------------------------------------------------------------------------------

m_ShadowDepthBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_ShadowDepthBuffer->SetName("CB_SpotLightShadow_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ShadowResourceViewDesc = {};
		ShadowResourceViewDesc.BufferLocation = m_ShadowDepthBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ShadowResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ShadowResourceViewDesc, m_ShadowDepthBuffer->GetCpuHandle());
	}

	void SpotLightSceneProxy::ReleaseShadowmap()
	{
		m_ShadowmapResource = nullptr;

		if (m_ShadowDepthBuffer)
		{
			m_ShadowDepthBuffer->ReleaseBufferedResource();
			m_ShadowDepthBuffer = nullptr;
		}
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