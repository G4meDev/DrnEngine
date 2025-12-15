#include "DrnPCH.h"
#include "EditorSelectionRenderBuffer.h"

namespace Drn
{
	EditorSelectionRenderBuffer::EditorSelectionRenderBuffer()
		: RenderBuffer()
		, m_DepthStencilTarget(nullptr)
	{}

	EditorSelectionRenderBuffer::~EditorSelectionRenderBuffer()
	{}

	void EditorSelectionRenderBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void EditorSelectionRenderBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		RenderResourceCreateInfo DepthStencilCreateInfo( nullptr, nullptr, ClearValueBinding::DepthZero, "DepthStencilTarget" );
		m_DepthStencilTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DEPTH_STENCIL_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), DepthStencilCreateInfo);


		D3D12_SHADER_RESOURCE_VIEW_DESC DsvSelectionDesc = {};
		//DsvSelectionDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		DsvSelectionDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		DsvSelectionDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		DsvSelectionDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		DsvSelectionDesc.Texture2D.PlaneSlice = 1;
		DsvSelectionDesc.Texture2D.MipLevels = 1;
		DsvSelectionDesc.Texture2D.MostDetailedMip = 0;
		
		m_StencilView = new ShaderResourceView(Renderer::Get()->GetDevice(), DsvSelectionDesc, m_DepthStencilTarget->m_ResourceLocation);

		// use this as another srv if needed depth
		//D3D12_SHADER_RESOURCE_VIEW_DESC DsvSelectionDesc = {};
		//DsvSelectionDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		//DsvSelectionDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//DsvSelectionDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//DsvSelectionDesc.Texture2D.PlaneSlice = 1;
		//DsvSelectionDesc.Texture2D.MipLevels = 1;
		//DsvSelectionDesc.Texture2D.MostDetailedMip = 0;
	}

	void EditorSelectionRenderBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearDepthTexture( m_DepthStencilTarget, EDepthStencilViewType::DepthStencilWrite, true, true );
	}

	void EditorSelectionRenderBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->SetViewport(0 ,0, 0, m_Size.X, m_Size.Y, 1);

		D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_DepthStencilTarget->GetDepthStencilView(EDepthStencilViewType::DepthStencilWrite)->GetView();
		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 0, nullptr, true, &Handle );
	}

}