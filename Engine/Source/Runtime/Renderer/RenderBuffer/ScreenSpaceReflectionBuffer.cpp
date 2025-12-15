#include "DrnPCH.h"
#include "ScreenSpaceReflectionBuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/HZBBuffer.h"

#define SSR_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT

namespace Drn
{
	ScreenSpaceReflectionBuffer::ScreenSpaceReflectionBuffer()
		: RenderBuffer()
		, m_Target(nullptr)
		, m_Buffer(nullptr)
	{ }

	ScreenSpaceReflectionBuffer::~ScreenSpaceReflectionBuffer()
	{
		ReleaseBuffers();
	}

	void ScreenSpaceReflectionBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

// ---------------------------------------------------------------------------------------------------------------

		m_Buffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_Buffer->SetName("CB_SSR");
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_Buffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_Buffer->GetCpuHandle());
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

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_Buffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_Data, sizeof(ScreenSpaceRefletcionData));
		m_Buffer->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void ScreenSpaceReflectionBuffer::ReleaseBuffers()
	{
		if (m_Buffer)
		{
			m_Buffer->ReleaseBufferedResource();
			m_Buffer = nullptr;
		}
	}

}