#include "DrnPCH.h"
#include "SpotLightSceneProxy.h"

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

// --------------------------------------------------------------------------------------------------

		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = 1;
		// TODO: make pooled and allocate on demand
		Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );
#if D3D12_Debug_INFO
		m_DsvHeap->SetName(StringHelper::s2ws("DsvHeapSpotLightShadowmap_" + m_Name).c_str());
#endif

		m_ShadowmapCpuHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();


// --------------------------------------------------------------------------------------------------

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

	void SpotLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
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

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

		// TODO: make light flags enum. e. g. 1: Pointlight. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 2;
		CommandList->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

		if (m_CastShadow)
		{
			ResourceStateTracker::Get()->TransiationResource(m_ShadowmapResource, D3D12_RESOURCE_STATE_DEPTH_READ);
			ResourceStateTracker::Get()->FlushResourceBarriers(CommandList);
		}

		CommonResources::Get()->m_SpotLightCone->BindAndDraw(CommandList);
	}

	void SpotLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{


		if (m_CastShadow)
		{
			D3D12_RECT R = CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX );
			CD3DX12_VIEWPORT Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)SPOTLIGHT_SHADOW_SIZE, (float)SPOTLIGHT_SHADOW_SIZE);

			CommandList->RSSetViewports(1, &Viewport);
			CommandList->RSSetScissorRects(1, &R);

			ResourceStateTracker::Get()->TransiationResource(m_ShadowmapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			ResourceStateTracker::Get()->FlushResourceBarriers(CommandList);

			CommandList->ClearDepthStencilView(m_ShadowmapCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
			CommandList->OMSetRenderTargets(0, nullptr, false, &m_ShadowmapCpuHandle);

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
			m_ShadowDepthData.ShadowmapTextureIndex = Renderer::Get()->GetBindlessSrvIndex(m_ShadowmapResource->GetGpuHandle());

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_ShadowDepthBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &m_ShadowDepthData, sizeof(SpotLightData));
			m_ShadowDepthBuffer->GetD3D12Resource()->Unmap(0, nullptr);

			CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
			CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ShadowDepthBuffer->GetGpuHandle()), 6);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, Renderer, this);
			}

		}
	}

	void SpotLightSceneProxy::AllocateShadowmap( ID3D12GraphicsCommandList2* CommandList )
	{
		D3D12_CLEAR_VALUE ShadowmapClearValue = {};
		ShadowmapClearValue.Format = DXGI_FORMAT_D16_UNORM;
		ShadowmapClearValue.DepthStencil.Depth = 1;

		m_ShadowmapResource = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16_TYPELESS, SPOTLIGHT_SHADOW_SIZE, SPOTLIGHT_SHADOW_SIZE, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, ShadowmapClearValue);

#if D3D12_Debug_INFO
		// TODO: add component name
		m_ShadowmapResource->SetName("SpotLightShadowmap");
#endif

		D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
		DepthViewDesc.Format = DXGI_FORMAT_D16_UNORM;
		DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DepthViewDesc.Texture2D.MipSlice = 0;
		Renderer::Get()->GetD3D12Device()->CreateDepthStencilView( m_ShadowmapResource->GetD3D12Resource(), &DepthViewDesc, m_ShadowmapCpuHandle );

// -----------------------------------------------------------------------------------------------------------

		D3D12_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		ResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		ResourceViewDesc.Format = DXGI_FORMAT_R16_UNORM;
		ResourceViewDesc.Texture2D.MipLevels = 1;
		ResourceViewDesc.Texture2D.MostDetailedMip = 0;
		ResourceViewDesc.Texture2D.ResourceMinLODClamp = 0;

		Renderer::Get()->GetD3D12Device()->CreateShaderResourceView(m_ShadowmapResource->GetD3D12Resource(), &ResourceViewDesc, m_ShadowmapResource->GetCpuHandle());

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
		if (m_ShadowmapResource)
		{
			m_ShadowmapResource->ReleaseBufferedResource();
			m_ShadowmapResource = nullptr;
		}

		if (m_ShadowDepthBuffer)
		{
			m_ShadowDepthBuffer->ReleaseBufferedResource();
			m_ShadowDepthBuffer = nullptr;
		}
	}

	void SpotLightSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
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