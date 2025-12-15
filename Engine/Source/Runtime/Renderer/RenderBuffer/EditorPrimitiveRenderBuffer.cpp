#include "DrnPCH.h"
#include "EditorPrimitiveRenderBuffer.h"

namespace Drn
{
	EditorPrimitiveRenderBuffer::EditorPrimitiveRenderBuffer()
		: RenderBuffer()
		, m_ColorTarget(nullptr)
		, m_DepthTarget(nullptr)
	{}

	EditorPrimitiveRenderBuffer::~EditorPrimitiveRenderBuffer()
	{}

	void EditorPrimitiveRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void EditorPrimitiveRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		RenderResourceCreateInfo ColorCreateInfo( nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "EditorPrimitiveColor" );
		m_ColorTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DISPLAY_OUTPUT_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), ColorCreateInfo);

		RenderResourceCreateInfo DepthCreateInfo( nullptr, nullptr, ClearValueBinding::DepthZero, "EditorPrimitiveDepth" );
		m_DepthTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DEPTH_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), DepthCreateInfo);
	}

	void EditorPrimitiveRenderBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_ColorTarget);
		//CommandList->ClearDepthStencilView( m_DepthCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0, 0, 0, nullptr );
	}

	void EditorPrimitiveRenderBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport(0 ,0, 0, m_Size.X, m_Size.Y, 1);

		D3D12_CPU_DESCRIPTOR_HANDLE ColorHandle = m_ColorTarget->GetRenderTargetView()->GetView();
		D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle = m_DepthTarget->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &ColorHandle, true, &DepthHandle );
	}

}