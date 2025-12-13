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
		, m_LightBuffer(nullptr)
		, m_ShadowDepthBuffer(nullptr)
		, m_ShadowCubemapResource(nullptr)
	{
		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_LightBuffer->SetName("CB_PointLight_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());
	}

	PointLightSceneProxy::~PointLightSceneProxy()
	{
		ReleaseShadowmap();
		ReleaseBuffer();
	}

	void PointLightSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		m_Buffer.WorldPosition = m_WorldPosition;
		m_Buffer.Scale = m_Radius;
		m_Buffer.Color = m_LightColor;
		m_Buffer.InvRadius = 1 / m_Radius;
		m_Buffer.ShadowBufferIndex = m_CastShadow ? Renderer::Get()->GetBindlessSrvIndex(m_ShadowDepthBuffer->GetGpuHandle()) : 0;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_LightBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_Buffer, sizeof(PointLightBuffer));
		m_LightBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

		// TODO: make light flags enum. e. g. 1: Point light. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 1;
		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

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
			D3D12_RECT R = CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX );

			CD3DX12_VIEWPORT Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(POINTLIGHT_SHADOW_SIZE), static_cast<float>(POINTLIGHT_SHADOW_SIZE));
			CommandList->GetD3D12CommandList()->RSSetViewports(1, &Viewport);
			CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &R);

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

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_ShadowDepthBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &m_ShadowDepthData, sizeof(ShadowDepthData));
			m_ShadowDepthBuffer->GetD3D12Resource()->Unmap(0, nullptr);

			CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
			CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ShadowDepthBuffer->GetGpuHandle()), 6);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, Renderer, this);
			}

		}
	}

	void PointLightSceneProxy::AllocateShadowmap( D3D12CommandList* CommandList )
	{
		RenderResourceCreateInfo ShadowmapCubemapCreateInfo( nullptr, nullptr, ClearValueBinding::DepthOne, "PointLightShadowmap" );
		m_ShadowCubemapResource = RenderTextureCube::Create(Renderer::Get()->GetCommandList_Temp(), POINTLIGHT_SHADOW_SIZE, DXGI_FORMAT_D16_UNORM, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), ShadowmapCubemapCreateInfo);

// -----------------------------------------------------------------------------------------------------------

		m_ShadowDepthBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 512 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_ShadowDepthBuffer->SetName("CB_PointLightShadow_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ShadowResourceViewDesc = {};
		ShadowResourceViewDesc.BufferLocation = m_ShadowDepthBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ShadowResourceViewDesc.SizeInBytes = 512;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ShadowResourceViewDesc, m_ShadowDepthBuffer->GetCpuHandle());
	}

	void PointLightSceneProxy::ReleaseShadowmap()
	{
		m_ShadowCubemapResource = nullptr;

		if (m_ShadowDepthBuffer)
		{
			m_ShadowDepthBuffer->ReleaseBufferedResource();
			m_ShadowDepthBuffer = nullptr;
		}
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
	{
		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}
	}

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