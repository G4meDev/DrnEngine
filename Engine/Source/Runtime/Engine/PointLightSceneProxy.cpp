#include "DrnPCH.h"
#include "PointLightSceneProxy.h"
#include "Runtime/Components/PointLightComponent.h"

#define POINTLIGHT_SHADOW_SIZE 512
#define POINTLIGHT_NEAR_Z 0.1f

namespace Drn
{
	PointLightSceneProxy::PointLightSceneProxy( class PointLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_Radius(InComponent->GetRadius())
		, m_DepthBias(InComponent->GetDepthBias())
		, m_ShadowCubemapResource(nullptr)
	{
		SetLocalToWorld(InComponent->GetLocalToWorld());

		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = 1;
		// TODO: make pooled and allocate on demand
		Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );

		m_ShadowmapCpuHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();

#if D3D12_Debug_INFO
		m_DsvHeap->SetName(StringHelper::s2ws("DsvHeapPointLightShadowmap_" + m_Name).c_str());
#endif

		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ);

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());

#if D3D12_Debug_INFO
		m_LightBuffer->SetName("ConstantBufferLight_" + m_Name);
#endif

		m_ShadowDepthBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 512 ), D3D12_RESOURCE_STATE_GENERIC_READ);

		D3D12_CONSTANT_BUFFER_VIEW_DESC ShadowResourceViewDesc = {};
		ShadowResourceViewDesc.BufferLocation = m_ShadowDepthBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ShadowResourceViewDesc.SizeInBytes = 512;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ShadowResourceViewDesc, m_ShadowDepthBuffer->GetCpuHandle());

#if D3D12_Debug_INFO
		m_ShadowDepthBuffer->SetName("ConstantBufferShadowDepth_" + m_Name);
#endif

		m_ShadowViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(POINTLIGHT_SHADOW_SIZE), static_cast<float>(POINTLIGHT_SHADOW_SIZE));
	}

	PointLightSceneProxy::~PointLightSceneProxy()
	{
		ReleaseShadowmap();

		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}

		if (m_ShadowDepthBuffer)
		{
			m_ShadowDepthBuffer->ReleaseBufferedResource();
			m_ShadowDepthBuffer = nullptr;
		}
	}

	void PointLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		Vector CameraPosition = Renderer->m_CameraActor->GetActorLocation();
		XMMATRIX LocalToWorld = XMMatrixTranslationFromVector( XMLoadFloat3( m_WorldPosition.Get() ) );
		XMMATRIX LocalToProjection = XMMatrixMultiply( m_LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );

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

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

		// TODO: make light flags enum. e. g. 1: Pointlight. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 1;
		CommandList->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

		//if (m_Sprite.IsValid())
		//{
		//	if (m_Sprite->IsRenderStateDirty())
		//	{
		//		m_Sprite->UploadResources(CommandList);
		//	}
		//
		//	CommandList->SetGraphicsRootDescriptorTable( 1, m_Sprite->TextureGpuHandle );
		//}

		CommonResources::Get()->m_PointLightSphere->BindAndDraw(CommandList);
	}

	void PointLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (m_CastShadow && !m_ShadowCubemapResource)
		{
			AllocateShadowmap(CommandList);
		}

		else if (!m_CastShadow && m_ShadowCubemapResource)
		{
			ReleaseShadowmap();
		}

		if (m_CastShadow)
		{
			D3D12_RECT R = CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX );

			CommandList->RSSetViewports(1, &m_ShadowViewport);
			CommandList->RSSetScissorRects(1, &R);

			CommandList->ClearDepthStencilView(m_ShadowmapCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
			CommandList->OMSetRenderTargets(0, nullptr, false, &m_ShadowmapCpuHandle);

			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[0], Vector::RightVector, Vector::UpVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[1], Vector::LeftVector, Vector::UpVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[2], Vector::UpVector, Vector::BackwardVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[3], Vector::DownVector, Vector::ForwardVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[4], Vector::ForwardVector, Vector::UpVector);
			CalculateLocalToProjectionForDirection(m_ShadowDepthData.WorldToProjectionMatrices[5], Vector::BackwardVector, Vector::UpVector);

			m_ShadowDepthData.DepthBias = m_DepthBias;
			m_ShadowDepthData.InvShadowResolution = 1.0f / POINTLIGHT_SHADOW_SIZE;
			m_ShadowDepthData.ShadowmapTextureIndex = Renderer::Get()->GetBindlessSrvIndex(m_ShadowCubemapResource->GetGpuHandle());

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_ShadowDepthBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &m_ShadowDepthData, sizeof(ShadowDepthData));
			m_ShadowDepthBuffer->GetD3D12Resource()->Unmap(0, nullptr);

			CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
			CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ShadowDepthBuffer->GetGpuHandle()), 6);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, Renderer, this);
			}

		}
	}

	void PointLightSceneProxy::AllocateShadowmap( ID3D12GraphicsCommandList2* CommandList )
	{
		D3D12_CLEAR_VALUE ShadowmapClearValue = {};
		ShadowmapClearValue.Format = DXGI_FORMAT_D16_UNORM;
		ShadowmapClearValue.DepthStencil.Depth = 1;

		m_ShadowCubemapResource = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16_TYPELESS, POINTLIGHT_SHADOW_SIZE, POINTLIGHT_SHADOW_SIZE, 6, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, ShadowmapClearValue);

#if D3D12_Debug_INFO
		// TODO: add component name
		m_ShadowCubemapResource->SetName("PointLightShadowmap");
#endif

		D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
		DepthViewDesc.Format = DXGI_FORMAT_D16_UNORM;
		DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		DepthViewDesc.Texture2DArray.MipSlice = 0;
		DepthViewDesc.Texture2DArray.ArraySize = 6;
		DepthViewDesc.Texture2DArray.FirstArraySlice = 0;
		Renderer::Get()->GetD3D12Device()->CreateDepthStencilView( m_ShadowCubemapResource->GetD3D12Resource(), &DepthViewDesc, m_ShadowmapCpuHandle );

// -----------------------------------------------------------------------------------------------------------

		D3D12_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		ResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		ResourceViewDesc.Format = DXGI_FORMAT_R16_UNORM;
		ResourceViewDesc.TextureCube.MipLevels = 1;
		ResourceViewDesc.TextureCube.MostDetailedMip = 0;
		ResourceViewDesc.TextureCube.ResourceMinLODClamp = 0;

		Renderer::Get()->GetD3D12Device()->CreateShaderResourceView(m_ShadowCubemapResource->GetD3D12Resource(), &ResourceViewDesc, m_ShadowCubemapResource->GetCpuHandle());
	}

	void PointLightSceneProxy::ReleaseShadowmap()
	{
		if (m_ShadowCubemapResource)
		{
			m_ShadowCubemapResource->ReleaseBufferedResource();
			m_ShadowCubemapResource = nullptr;
		}
	}

#if WITH_EDITOR
	void PointLightSceneProxy::DrawAttenuation(World* InWorld)
	{
		InWorld->DrawDebugSphere( m_WorldPosition, Quat::Identity, Color::White, m_Radius, 36, 0.0, 0 );
	}
#endif

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