#include "DrnPCH.h"
#include "RenderBufferAO.h"
#include "Runtime/Renderer/SceneRenderer.h"
#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/HZBBuffer.h"

#define AO_FORMAT DXGI_FORMAT_R8_UNORM
#define AO_SETUP_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT

namespace Drn
{
	RenderBufferAO::RenderBufferAO()
		: RenderBuffer()
		, m_AOTarget(nullptr)
		, m_AOHalfTarget(nullptr)
		, m_AOSetupTarget(nullptr)
		, m_AoBuffer(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		
	}

	RenderBufferAO::~RenderBufferAO()
	{
		ReleaseBuffers();
	}

	void RenderBufferAO::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		{
			D3D12_DESCRIPTOR_HEAP_DESC AOHeapDesc = {};
			AOHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			AOHeapDesc.NumDescriptors = 3;
			Device->CreateDescriptorHeap( &AOHeapDesc, IID_PPV_ARGS(m_AORtvHeap.ReleaseAndGetAddressOf()) );

			m_AOHalfHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_AORtvHeap->GetCPUDescriptorHandleForHeapStart(), 0, Renderer::Get()->GetRtvIncrementSize());
			m_AOSetupHandle	= CD3DX12_CPU_DESCRIPTOR_HANDLE(m_AORtvHeap->GetCPUDescriptorHandleForHeapStart(), 1, Renderer::Get()->GetRtvIncrementSize());
			m_AOHandle	= CD3DX12_CPU_DESCRIPTOR_HANDLE(m_AORtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, Renderer::Get()->GetRtvIncrementSize());
#if D3D12_Debug_INFO
			m_AORtvHeap->SetName(L"RtvHeap_AO");
#endif
		}
	}

	void RenderBufferAO::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		IntPoint HalfSize = IntPoint::ComponentWiseMax(Size / 2, IntPoint(1));
		m_SetupViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(HalfSize.X), static_cast<float>(HalfSize.Y));

		ReleaseBuffers();

		{
			m_AOTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(AO_FORMAT, m_Size.X, m_Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = AO_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_AOTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_AOHandle );

#if D3D12_Debug_INFO
			m_AOTarget->SetName( "AOTarget" );
#endif

			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = AO_FORMAT;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView(m_AOTarget->GetD3D12Resource(), &Desc, m_AOTarget->GetCpuHandle());
		}

		{
			m_AOHalfTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(AO_FORMAT, HalfSize.X, HalfSize.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = AO_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_AOHalfTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_AOHalfHandle );

#if D3D12_Debug_INFO
			m_AOHalfTarget->SetName( "AOHalfTarget" );
#endif

			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = AO_FORMAT;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView(m_AOHalfTarget->GetD3D12Resource(), &Desc, m_AOHalfTarget->GetCpuHandle());
		}

		{
			m_AOSetupTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
				CD3DX12_RESOURCE_DESC::Tex2D(AO_SETUP_FORMAT, HalfSize.X, HalfSize.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
				D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = {};
			RenderTargetViewDesc.Format = AO_SETUP_FORMAT;
			RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RenderTargetViewDesc.Texture2D.MipSlice = 0;

			Device->CreateRenderTargetView( m_AOSetupTarget->GetD3D12Resource(), &RenderTargetViewDesc, m_AOSetupHandle );

#if D3D12_Debug_INFO
			m_AOSetupTarget->SetName( "AOSetupTarget" );
#endif

			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = AO_SETUP_FORMAT;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = 0;

			Device->CreateShaderResourceView(m_AOSetupTarget->GetD3D12Resource(), &Desc, m_AOSetupTarget->GetCpuHandle());
		}

		{
			m_AoBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
			m_AoBuffer->SetName("CB_AoBuffer_");
#endif

			D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
			ResourceViewDesc.BufferLocation = m_AoBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
			ResourceViewDesc.SizeInBytes = 256;
			Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_AoBuffer->GetCpuHandle());
		}
	}

	void RenderBufferAO::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void RenderBufferAO::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
	}

	void RenderBufferAO::BindSetup( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_SetupViewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);
		
		CommandList->OMSetRenderTargets( 1, &m_AOSetupHandle, true, nullptr );
	}

	void RenderBufferAO::BindHalf( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_SetupViewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);
		
		CommandList->OMSetRenderTargets( 1, &m_AOHalfHandle, true, nullptr );

	}

	void RenderBufferAO::BindMain( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);
		
		CommandList->OMSetRenderTargets( 1, &m_AOHandle, true, nullptr );
	}

	void RenderBufferAO::MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer, const SSAOSettings& Settings )
	{
		m_AoData.DepthTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_DepthTarget->GetGpuHandle());
		m_AoData.WorldNormalTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_GBuffer->m_WorldNormalTarget->GetGpuHandle());
		m_AoData.HzbTexture = Renderer::Get()->GetBindlessSrvIndex(Renderer->m_HZBBuffer->M_HZBTarget->GetGpuHandle());
		m_AoData.SetupTexture = Renderer::Get()->GetBindlessSrvIndex(m_AOSetupTarget->GetGpuHandle());
		m_AoData.DownSampleTexture = Renderer::Get()->GetBindlessSrvIndex(m_AOHalfTarget->GetGpuHandle());

		Texture2D* SSAO_RandomTexture = CommonResources::Get()->m_SSAO_Random.Get();
		m_AoData.RandomTexture = SSAO_RandomTexture->GetTextureIndex();

		m_AoData.ToRandomU = (float) m_Size.X / SSAO_RandomTexture->GetSizeX();
		m_AoData.ToRandomV = (float) m_Size.Y / SSAO_RandomTexture->GetSizeY();

		m_AoData.Intensity = Settings.m_Intensity;
		m_AoData.Power = Settings.m_Power;
		m_AoData.Bias = Settings.m_Bias;
		m_AoData.Radius = Settings.m_Radius;
		m_AoData.MipBlend = Settings.m_MipBlend;

		float FadeRadius = std::max( 1.0f, Settings.m_FadeRadius );
		float InvFadeRadius = 1.0f / FadeRadius;
		float FadeOffset = -(Settings.m_FadeDistance - FadeRadius) * InvFadeRadius;

		m_AoData.InvFadeRadius = InvFadeRadius;
		m_AoData.FadeOffset = FadeOffset;


		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_AoBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_AoData, sizeof(AoData));
		m_AoBuffer->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void RenderBufferAO::ReleaseBuffers()
	{
		if (m_AOTarget)
			m_AOTarget->ReleaseBufferedResource();

		if (m_AOHalfTarget)
			m_AOHalfTarget->ReleaseBufferedResource();

		if (m_AOSetupTarget)
			m_AOSetupTarget->ReleaseBufferedResource();

		if (m_AoBuffer)
			m_AoBuffer->ReleaseBufferedResource();
	}

}