#include "DrnPCH.h"
#include "PointLightSceneProxy.h"
#include "Runtime/Components/PointLightComponent.h"

#define POINTLIGHT_SHADOW_SIZE 512

namespace Drn
{
	PointLightSceneProxy::PointLightSceneProxy( class PointLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_Radius(InComponent->GetRadius())
		, m_ShadowCubemapResource(nullptr)
	{
		SetLocalToWorld(InComponent->GetLocalToWorld());

		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = 1;
		// TODO: make pooled and allocate on demand
		Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );

		m_ShadowmapCpuHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();

		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ);

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());
	}

	PointLightSceneProxy::~PointLightSceneProxy()
	{
		ReleaseShadowmap();

		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}
	}

	void PointLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		Vector CameraPosition = Renderer->m_CameraActor->GetActorLocation();
		XMMATRIX LocalToWorld = XMMatrixTranslationFromVector( XMLoadFloat3( m_WorldPosition.Get() ) );
		XMMATRIX LocalToProjection = XMMatrixMultiply( m_LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );

		m_Buffer.LocalToProjection = LocalToProjection;
		m_Buffer.CameraPosition = CameraPosition;
		m_Buffer.WorldPosition = m_WorldPosition;
		m_Buffer.Radius = m_Radius;
		m_Buffer.LightColor = m_LightColor;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_LightBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_Buffer, sizeof(PointLightBuffer));
		m_LightBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

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
			CommandList->ClearDepthStencilView(m_ShadowmapCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
			CommandList->OMSetRenderTargets(0, nullptr, false, &m_ShadowmapCpuHandle);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, this);
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

		std::cout << "Allocated";
	}

	void PointLightSceneProxy::ReleaseShadowmap()
	{
		std::cout << "Released";

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
}