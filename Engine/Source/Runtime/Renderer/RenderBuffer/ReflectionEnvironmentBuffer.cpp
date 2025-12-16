#include "DrnPCH.h"
#include "ReflectionEnvironmentBuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/ScreenSpaceReflectionBuffer.h"
#include "Runtime/Renderer/RenderBuffer/RenderBufferAO.h"

#include "Runtime/Engine/SkyLightSceneProxy.h"

namespace Drn
{
	ReflectionEnvironmentBuffer::ReflectionEnvironmentBuffer()
	{}

	ReflectionEnvironmentBuffer::~ReflectionEnvironmentBuffer()
	{}

	void ReflectionEnvironmentBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

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

	void ReflectionEnvironmentBuffer::MapBuffer(D3D12CommandList* CommandList, SceneRenderer* Renderer)
	{
		m_Data.BaseColorTexture = Renderer->m_GBuffer->m_BaseColorTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.WorldNormalTexture = Renderer->m_GBuffer->m_WorldNormalTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.MasksTexture = Renderer->m_GBuffer->m_MasksTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.DepthTexture = Renderer->m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.SSRTexture = Renderer->m_ScreenSpaceReflectionBuffer->m_Target->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.PreintegratedGFTexture = CommonResources::Get()->m_PreintegratedGF->GetTextureIndex();
		m_Data.AOTexture = Renderer->m_AOBuffer->m_AOTarget->GetShaderResourceView()->GetDescriptorHeapIndex();

		SkyLightSceneProxy* SkyProxy = Renderer->GetScene()->m_SkyLightProxies.size() > 0 ? *Renderer->GetScene()->m_SkyLightProxies.begin() : nullptr;
		AssetHandle<TextureCube> SkyCubemap = SkyProxy ? SkyProxy->GetCubemap() : AssetHandle<TextureCube>();
		m_Data.SkyCubemapTexture = SkyCubemap.IsValid() ? SkyCubemap->GetTextureIndex() : 0;
		m_Data.SkyLightMipCount = SkyCubemap.IsValid() ? SkyCubemap->GetMipLevels() : 0;
		m_Data.SkyLightColor = SkyProxy ? SkyProxy->GetColor() : Vector::ZeroVector;

		Buffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(ReflectionEnvironmentData), EUniformBufferUsage::SingleFrame, &m_Data);
	}

}