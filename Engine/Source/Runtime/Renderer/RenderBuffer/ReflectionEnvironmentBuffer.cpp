#include "DrnPCH.h"
#include "ReflectionEnvironmentBuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/ScreenSpaceReflectionBuffer.h"
#include "Runtime/Renderer/RenderBuffer/RenderBufferAO.h"

#include "Runtime/Engine/SkyLightSceneProxy.h"

namespace Drn
{
	ReflectionEnvironmentBuffer::ReflectionEnvironmentBuffer()
		: m_Buffer(nullptr)
	{
		
	}

	ReflectionEnvironmentBuffer::~ReflectionEnvironmentBuffer()
	{
		ReleaseBuffers();
	}

	void ReflectionEnvironmentBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		m_Buffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_Buffer->SetName("CB_ReflectionEnvironment");
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_Buffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_Buffer->GetCpuHandle());
	}

	void ReflectionEnvironmentBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
	}

	void ReflectionEnvironmentBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void ReflectionEnvironmentBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void ReflectionEnvironmentBuffer::MapBuffer(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer)
	{
		m_Data.BaseColorTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_BaseColorTarget->GetGpuHandle());
		m_Data.WorldNormalTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_WorldNormalTarget->GetGpuHandle());
		m_Data.MasksTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_MasksTarget->GetGpuHandle());
		m_Data.DepthTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_DepthTarget->GetGpuHandle());
		m_Data.SSRTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_ScreenSpaceReflectionBuffer->m_Target->GetGpuHandle());
		m_Data.PreintegratedGFTexture = Renderer::Get()->GetBindlessSrvIndex(CommonResources::Get()->m_PreintegratedGF->GetResource()->GetGpuHandle());
		m_Data.AOTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_AOBuffer->m_AOTarget->GetGpuHandle());

		SkyLightSceneProxy* SkyProxy = Renderer->GetScene()->m_SkyLightProxies.size() > 0 ? *Renderer->GetScene()->m_SkyLightProxies.begin() : nullptr;
		AssetHandle<TextureCube> SkyCubemap = SkyProxy ? SkyProxy->GetCubemap() : AssetHandle<TextureCube>();
		m_Data.SkyCubemapTexture = SkyCubemap.IsValid() ? Renderer::Get()->GetBindlessSrvIndex(SkyCubemap->GetResource()->GetGpuHandle()) : 0;
		m_Data.SkyLightMipCount = SkyCubemap.IsValid() ? SkyCubemap->GetMipLevels() : 0;
		m_Data.SkyLightColor = SkyProxy ? SkyProxy->GetColor() : Vector::OneVector;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_Buffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_Data, sizeof(ReflectionEnvironmentData));
		m_Buffer->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void ReflectionEnvironmentBuffer::ReleaseBuffers()
	{
		if (m_Buffer)
		{
			m_Buffer->ReleaseBufferedResource();
			m_Buffer = nullptr;
		}
	}

}