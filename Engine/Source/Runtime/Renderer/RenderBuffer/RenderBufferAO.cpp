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

	}

	void RenderBufferAO::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		HalfSize = IntPoint::ComponentWiseMax(m_Size / 2, IntPoint(1));

		ReleaseBuffers();

		RenderResourceCreateInfo AOTargetCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "AOTarget" );
		m_AOTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, AO_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), AOTargetCreateInfo);

		RenderResourceCreateInfo AOHalfTargetCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "AOHalfTarget" );
		m_AOHalfTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), HalfSize.X, HalfSize.Y, AO_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), AOHalfTargetCreateInfo);

		RenderResourceCreateInfo AOSetupTargetCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "AOSetupTarget" );
		m_AOSetupTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), HalfSize.X, HalfSize.Y, AO_SETUP_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), AOSetupTargetCreateInfo);

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

	void RenderBufferAO::Clear( D3D12CommandList* CommandList )
	{
		
	}

	void RenderBufferAO::Bind( D3D12CommandList* CommandList )
	{
	}

	void RenderBufferAO::BindSetup( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport( 0, 0, 0, HalfSize.X, HalfSize.Y, 1 );

		D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_AOSetupTarget->GetRenderTargetView()->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &Handle, true, nullptr );
	}

	void RenderBufferAO::BindHalf( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport( 0, 0, 0, HalfSize.X, HalfSize.Y, 1 );
		
		D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_AOHalfTarget->GetRenderTargetView()->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &Handle, true, nullptr );
	}

	void RenderBufferAO::BindMain( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport( 0, 0, 0, m_Size.X, m_Size.Y, 1 );
		
		D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_AOTarget->GetRenderTargetView()->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &Handle, true, nullptr );
	}

	void RenderBufferAO::MapBuffer( D3D12CommandList* CommandList, SceneRenderer* Renderer, const SSAOSettings& Settings )
	{
		m_AoData.DepthTexture = Renderer->m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_AoData.WorldNormalTexture = Renderer->m_GBuffer->m_WorldNormalTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_AoData.HzbTexture = Renderer->m_HZBBuffer->M_HZBTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_AoData.SetupTexture = m_AOSetupTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_AoData.DownSampleTexture = m_AOHalfTarget->GetShaderResourceView()->GetDescriptorHeapIndex();

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

		const float TemporalScalar = (float)Renderer->GetSceneView().FrameIndexMod8 / SSAO_RandomTexture->GetSizeX();
		m_AoData.TemporalOffset = Vector2(TemporalScalar * 2.48f, TemporalScalar * 7.52f);

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_AoBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_AoData, sizeof(AoData));
		m_AoBuffer->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void RenderBufferAO::ReleaseBuffers()
	{
		if (m_AoBuffer)
			m_AoBuffer->ReleaseBufferedResource();
	}

}