#include "DrnPCH.h"
#include "DirectionalLightSceneProxy.h"
#include "Runtime/Components/DirectionalLightComponent.h"

#define DIRECTIONAL_SHADOW_SIZE 2048
#define DIRECTIONAL_SHADOW_WIDTH 128.0f
#define DIRECTIONAL_SHADOW_NEARZ 0.1f
#define DIRECTIONAL_SHADOW_FARZ 100.0f

namespace Drn
{
	DirectionalLightSceneProxy::DirectionalLightSceneProxy( class DirectionalLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_LightBuffer(nullptr)
		, m_ShadowBuffer(nullptr)
		, m_ShadowmapResource(nullptr)
	{
		m_DirectionalLightComponent = InComponent;
		m_LightData.Direction = InComponent->GetWorldRotation().GetVector();

// --------------------------------------------------------------------------------------------------

		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = 1;
		// TODO: make pooled and allocate on demand
		Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );
#if D3D12_Debug_INFO
		m_DsvHeap->SetName(StringHelper::s2ws("DsvHeapDirectionalLightShadowmap_" + m_Name).c_str());
#endif

		m_ShadowmapCpuHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();


// --------------------------------------------------------------------------------------------------

		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_LightBuffer->SetName("CB_DircetionalLight_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());


// --------------------------------------------------------------------------------------------------

		m_ShadowBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_ShadowBuffer->SetName("CB_DirectionalLightShadow_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ShadowResourceViewDesc = {};
		ShadowResourceViewDesc.BufferLocation = m_ShadowBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ShadowResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ShadowResourceViewDesc, m_ShadowBuffer->GetCpuHandle());
	}

	DirectionalLightSceneProxy::~DirectionalLightSceneProxy()
	{
		ReleaseShadowmap();
		ReleaseBuffers();
	}

	void DirectionalLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		// TODO: remove. should not be aware of component. lazy update
		m_LightData.Direction = m_DirectionalLightComponent->GetWorldRotation().GetVector();
		m_LightData.Color = m_DirectionalLightComponent->GetScaledColor();
		m_LightData.ShadowmapBufferIndex = m_CastShadow ? Renderer::Get()->GetBindlessSrvIndex(m_ShadowBuffer->GetGpuHandle()) : 0;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_LightBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_LightData, sizeof(DirectionalLightData));
		m_LightBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

		// TODO: make light flags enum. e. g. 1: Pointlight. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 4;
		CommandList->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

		if (m_CastShadow)
		{
			ResourceStateTracker::Get()->TransiationResource(m_ShadowmapResource, D3D12_RESOURCE_STATE_DEPTH_READ);
			ResourceStateTracker::Get()->FlushResourceBarriers(CommandList);
		}

		CommonResources::Get()->m_BackfaceScreenTriangle->BindAndDraw(CommandList);
	}

	void DirectionalLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (m_CastShadow && !m_ShadowmapResource)
		{
			AllocateShadowmap(CommandList);
		}

		else if (!m_CastShadow && m_ShadowmapResource)
		{
			ReleaseShadowmap();
		}

		if (m_CastShadow)
		{
			D3D12_RECT R = CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX );
			CD3DX12_VIEWPORT Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)DIRECTIONAL_SHADOW_SIZE, (float)DIRECTIONAL_SHADOW_SIZE);

			CommandList->RSSetViewports(1, &Viewport);
			CommandList->RSSetScissorRects(1, &R);

			ResourceStateTracker::Get()->TransiationResource(m_ShadowmapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			ResourceStateTracker::Get()->FlushResourceBarriers(CommandList);

			CommandList->ClearDepthStencilView(m_ShadowmapCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
			CommandList->OMSetRenderTargets(0, nullptr, false, &m_ShadowmapCpuHandle);

// ----------------------------------------------------------------------------------------

			Vector LightPosition = Renderer->GetSceneView().CameraPos;
			Vector ViewDirection = m_LightData.Direction;
			LightPosition = LightPosition + ViewDirection * DIRECTIONAL_SHADOW_FARZ * -0.5f;
			Vector FocusPoint = LightPosition + ViewDirection;

			Matrix ViewMatrix = XMMatrixLookAtLH( XMLoadFloat3(LightPosition.Get()), XMLoadFloat3(FocusPoint.Get()), XMLoadFloat3(Vector::UpVector.Get()));
			// TODO: make zfar param
			Matrix ProjectionMatrix = XMMatrixOrthographicLH(DIRECTIONAL_SHADOW_WIDTH, DIRECTIONAL_SHADOW_WIDTH, DIRECTIONAL_SHADOW_NEARZ, DIRECTIONAL_SHADOW_FARZ);

			Matrix ViewProjection = ViewMatrix * ProjectionMatrix;
			m_ShadowData.WorldToProjectionMatrices = ViewProjection;

// ----------------------------------------------------------------------------------------

			m_ShadowData.DepthBias = m_DirectionalLightComponent->GetDepthBias();
			m_ShadowData.InvShadowResolution = 1.0f / DIRECTIONAL_SHADOW_SIZE;
			m_ShadowData.ShadowmapTextureIndex = Renderer::Get()->GetBindlessSrvIndex(m_ShadowmapResource->GetGpuHandle());

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_ShadowBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &m_ShadowData, sizeof(DirectionalLightShadowData));
			m_ShadowBuffer->GetD3D12Resource()->Unmap(0, nullptr);

			CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
			CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ShadowBuffer->GetGpuHandle()), 6);

			for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
			{
				Proxy->RenderShadowPass(CommandList, Renderer, this);
			}
		}

	}

	void DirectionalLightSceneProxy::AllocateShadowmap( ID3D12GraphicsCommandList2* CommandList )
	{
		D3D12_CLEAR_VALUE ShadowmapClearValue = {};
		ShadowmapClearValue.Format = DXGI_FORMAT_D16_UNORM;
		ShadowmapClearValue.DepthStencil.Depth = 1;

		m_ShadowmapResource = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16_TYPELESS, DIRECTIONAL_SHADOW_SIZE, DIRECTIONAL_SHADOW_SIZE, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, ShadowmapClearValue);

#if D3D12_Debug_INFO
		// TODO: add component name
		m_ShadowmapResource->SetName("DirectionalLightShadowmap");
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
	}

	void DirectionalLightSceneProxy::ReleaseShadowmap()
	{
		if (m_ShadowmapResource)
		{
			m_ShadowmapResource->ReleaseBufferedResource();
			m_ShadowmapResource = nullptr;
		}
	}

	void DirectionalLightSceneProxy::ReleaseBuffers()
	{
		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}

		if (m_ShadowBuffer)
		{
			m_ShadowBuffer->ReleaseBufferedResource();
			m_ShadowBuffer = nullptr;
		}
	}

#if WITH_EDITOR
	void DirectionalLightSceneProxy::DrawAttenuation( World* InWorld )
	{
		InWorld->DrawDebugArrow(m_DirectionalLightComponent->GetWorldLocation(),
			m_DirectionalLightComponent->GetWorldLocation() + m_DirectionalLightComponent->GetWorldRotation().GetVector() * 1.2f, 0.1f, Color::White, 0.0f, 0.0f);
	}
#endif
}