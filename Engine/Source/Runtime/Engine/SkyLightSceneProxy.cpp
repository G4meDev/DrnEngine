#include "DrnPCH.h"
#include "SkyLightSceneProxy.h"
#include "Runtime/Components/SkyLightComponent.h"

namespace Drn
{
	SkyLightSceneProxy::SkyLightSceneProxy( class SkyLightComponent* InComponent )
		: LightSceneProxy( InComponent )
		, m_SkyLightComponent(InComponent)
		, m_LightBuffer(nullptr)
		, m_Cubemap("")
		, m_BlockLowerHemesphere(false)
		, m_LowerHemesphereColor()
	{
		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_LightBuffer->SetName("CB_SkyLight_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());
	}

	SkyLightSceneProxy::~SkyLightSceneProxy()
	{
		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}
	}

	void SkyLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		m_SkyLightData.Color = m_LightColor;
		m_SkyLightData.BlockLowerHemesphere = m_BlockLowerHemesphere;
		m_SkyLightData.LowerHemesphereColor = m_LowerHemesphereColor;
		m_SkyLightData.CubemapTexture = m_Cubemap.IsValid() ? Renderer::Get()->GetBindlessSrvIndex(m_Cubemap->GetResource()->GetGpuHandle()) : 0;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_LightBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_SkyLightData, sizeof(SkyLightData));
		m_LightBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);

		// TODO: make light flags enum. e. g. 1: Pointlight. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 8;
		CommandList->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

		CommonResources::Get()->m_BackfaceScreenTriangle->BindAndDraw(CommandList);
	}

	void SkyLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
	}

	void SkyLightSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
	{
		if (m_SkyLightComponent && m_SkyLightComponent->IsRenderStateDirty())
		{
			m_SkyLightComponent->ClearRenderStateDirty();

			m_LightColor = m_SkyLightComponent->GetScaledColor();
			m_Cubemap = m_SkyLightComponent->GetCubemap();
			m_BlockLowerHemesphere = m_SkyLightComponent->GetBlockLowerHemisphere();
			m_LowerHemesphereColor = m_SkyLightComponent->GetLowerHemisphereColor();
		}

		if (m_Cubemap.IsValid())
		{
			m_Cubemap->UploadResources(CommandList);
		}
	}

}