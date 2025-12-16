#include "DrnPCH.h"
#include "ScreenSpaceReflectionBuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/HZBBuffer.h"

#define SSR_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT

namespace Drn
{
	ScreenSpaceReflectionBuffer::ScreenSpaceReflectionBuffer()
		: RenderBuffer()
	{}

	ScreenSpaceReflectionBuffer::~ScreenSpaceReflectionBuffer()
	{}

	void ScreenSpaceReflectionBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void ScreenSpaceReflectionBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		RenderResourceCreateInfo SSRTargetCreateInfo( nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "SSRTarget" );
		m_Target = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, SSR_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), SSRTargetCreateInfo);
	}

	void ScreenSpaceReflectionBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_Target);
	}

	void ScreenSpaceReflectionBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport( 0, 0, 0, m_Size.X, m_Size.Y, 1 );

		D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_Target->GetRenderTargetView()->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &Handle, true, nullptr );
	}

	void ScreenSpaceReflectionBuffer::MapBuffer( D3D12CommandList* CommandList, SceneRenderer* Renderer, const SSRSettings& Settings )
	{
		m_Data.DeferredColorTexture = Renderer->m_GBuffer->m_ColorDeferredTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.BaseColorTexture = Renderer->m_GBuffer->m_BaseColorTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.WorldNormalTexture = Renderer->m_GBuffer->m_WorldNormalTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.MasksTexture = Renderer->m_GBuffer->m_MasksTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.DepthTexture = Renderer->m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.HzbTexture = Renderer->m_HZBBuffer->M_HZBTarget->GetShaderResourceView()->GetDescriptorHeapIndex();

		m_Data.Intensity = Settings.m_Intensity;
		m_Data.RoughnessFade = Settings.m_RoughnessFade;

		Buffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(ScreenSpaceRefletcionData), EUniformBufferUsage::SingleFrame, &m_Data);
	}

}