#include "DrnPCH.h"
#include "DecalBuffer.h"

namespace Drn
{
	DecalBuffer::DecalBuffer()
		: RenderBuffer()
		, m_BaseColorTarget(nullptr)
		, m_NormalTarget(nullptr)
		, m_MasksTarget(nullptr)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{}

	DecalBuffer::~DecalBuffer()
	{}

	void DecalBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void DecalBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));

		RenderResourceCreateInfo BaseColorCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "Decal_BaseColor" );
		m_BaseColorTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DECAL_BASE_COLOR_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), BaseColorCreateInfo);

		RenderResourceCreateInfo NormalCreateInfo( nullptr, nullptr, ClearValueBinding(Vector4(0.5f, 0.5f, 1.0f, 1.0f)), "Decal_Normal" );
		m_NormalTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DECAL_NORMAL_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), NormalCreateInfo);

		RenderResourceCreateInfo MasksCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "Decal_Masks" );
		m_MasksTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_Size.X, m_Size.Y, DECAL_MASKS_FORMAT, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), MasksCreateInfo);
	}

	void DecalBuffer::Clear( D3D12CommandList* CommandList )
	{
		CommandList->ClearColorTexture(m_BaseColorTarget);
		CommandList->ClearColorTexture(m_NormalTarget);
		CommandList->ClearColorTexture(m_MasksTarget);
	}

	void DecalBuffer::Bind( D3D12CommandList* CommandList )
	{
		CommandList->GetD3D12CommandList()->RSSetViewports(1, &m_Viewport);
		CommandList->GetD3D12CommandList()->RSSetScissorRects(1, &m_ScissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE const RenderTargets[3] = 
		{
			m_BaseColorTarget->GetRenderTargetView()->GetView(),
			m_NormalTarget->GetRenderTargetView()->GetView(),
			m_MasksTarget->GetRenderTargetView()->GetView()
		};

		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 3, RenderTargets, false, NULL );
	}

}