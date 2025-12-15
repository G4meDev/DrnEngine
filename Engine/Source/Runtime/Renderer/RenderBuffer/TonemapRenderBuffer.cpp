#include "DrnPCH.h"
#include "TonemapRenderBuffer.h"

namespace Drn
{
	TonemapRenderBuffer::TonemapRenderBuffer()
		: RenderBuffer()
		, m_TonemapTarget(nullptr)
	{}

	TonemapRenderBuffer::~TonemapRenderBuffer()
	{}

	void TonemapRenderBuffer::Init()
	{
		RenderBuffer::Init();
	}

	void TonemapRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		RenderResourceCreateInfo TonemapTargetCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "TonemapTarget" );
		m_TonemapTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DISPLAY_OUTPUT_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), TonemapTargetCreateInfo);
	}

	void TonemapRenderBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_TonemapTarget);
	}

	void TonemapRenderBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport( 0, 0, 0, m_Size.X, m_Size.Y, 1 );

		D3D12_CPU_DESCRIPTOR_HANDLE TonemapHandle = m_TonemapTarget->GetRenderTargetView()->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &TonemapHandle, true, nullptr );
	}

}