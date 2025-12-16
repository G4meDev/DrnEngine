#include "DrnPCH.h"
#include "TAABuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"

namespace Drn
{
	Vector2 TAABuffer::m_JitterOffsets[8] = 
	{
		{0.0, -0.33334},
		{-0.5, 0.333334},
		{0.5f, -0.777778},
		{-0.75, -0.111112},
		{0.25, 0.555556},
		{-0.25, -0.555556},
		{ 0.75, 0.111112},
		{-0.875, 0.777778},
	};

	//Vector TAABuffer::m_JitterOffsets[4][2] = 
	//{
	//	{-0.5f, -0.5f},
	//	{0.5f, -0.5f},
	//	{0.5f, 0.5f},
	//	{-0.5f, 0.5f}
	//};

	//Vector TAABuffer::m_JitterOffsets[2][2] = 
	//{
	//	{0.5f, 0.0f},
	//	{0.0f, 0.5f}
	//};

	TAABuffer::TAABuffer()
		: RenderBuffer()
	{}

	TAABuffer::~TAABuffer()
	{}

	void TAABuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void TAABuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		for (int32 i = 0; i < 2; i++)
		{
			RenderResourceCreateInfo TaaTargetCreateInfo( nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "TAATarget_" + std::to_string(i) );
			m_TAATarget[i] = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, GBUFFER_COLOR_DEFERRED_FORMAT, 1, 1, true,
				(ETextureCreateFlags)(ETextureCreateFlags::UAV | ETextureCreateFlags::ShaderResource), TaaTargetCreateInfo);

			D3D12_UNORDERED_ACCESS_VIEW_DESC DescUAV = {};
			DescUAV.Format = GBUFFER_COLOR_DEFERRED_FORMAT;
			DescUAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			DescUAV.Texture2D.MipSlice = 0;
			DescUAV.Texture2D.PlaneSlice = 0;

			m_UavViews[i] = new UnorderedAccessView( Renderer::Get()->GetDevice(), DescUAV, m_TAATarget[i]->m_ResourceLocation );
		}
	}

	void TAABuffer::Clear( D3D12CommandList* CommandList )
	{
		
	}

	void TAABuffer::Bind( D3D12CommandList* CommandList )
	{
	}

	void TAABuffer::MapBuffer( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		m_Data.DeferredColorTexture = Renderer->m_GBuffer->m_ColorDeferredTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.VelocityTexture = Renderer->m_GBuffer->m_VelocityTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.HistoryTexture = GetHistoryResource(Renderer->m_SceneView.FrameIndex)->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.TargetTexture = GetFrameUAV(Renderer->m_SceneView.FrameIndex)->GetDescriptorHeapIndex();

		m_Data.DepthTexture = Renderer->m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.CurrentFrameWeight = Renderer->m_PostProcessSettings->m_TAASettings.m_CurrentFrameWeight;
		m_Data.CcurrentFrameVelocityWeight = Renderer->m_PostProcessSettings->m_TAASettings.m_CcurrentFrameVelocityWeight;
		m_Data.CcurrentFrameVelocityMultiplier = Renderer->m_PostProcessSettings->m_TAASettings.m_CcurrentFrameVelocityMultiplier;

		Buffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(TAAData), EUniformBufferUsage::SingleFrame, &m_Data);
	}

	
}