#include "DrnPCH.h"
#include "DecalBuffer.h"

namespace Drn
{
	DecalBuffer::DecalBuffer()
		: RenderBuffer()
		, m_BaseColorTarget(nullptr)
		, m_NormalTarget(nullptr)
		, m_MasksTarget(nullptr)
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
		CommandList->SetViewport(0 ,0, 0, m_Size.X, m_Size.Y, 1);

		D3D12_CPU_DESCRIPTOR_HANDLE const RenderTargets[3] = 
		{
			m_BaseColorTarget->GetRenderTargetView()->GetView(),
			m_NormalTarget->GetRenderTargetView()->GetView(),
			m_MasksTarget->GetRenderTargetView()->GetView()
		};

		CommandList->GetD3D12CommandList()->OMSetRenderTargets( 3, RenderTargets, false, NULL );
	}

}