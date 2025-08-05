#include "DrnPCH.h"
#include "HZBBuffer.h"

#define HZB_FORMAT DXGI_FORMAT_R16_FLOAT

namespace Drn
{
	HZBBuffer::HZBBuffer()
		: RenderBuffer()
		, M_HZBTarget(nullptr)
		, m_FirstMipSize(1)
		, m_MipCount(1)
	{
		m_ClearValue.Format   = HZB_FORMAT;
		m_ClearValue.Color[0] = 0;
		m_ClearValue.Color[1] = 0;
		m_ClearValue.Color[2] = 0;
		m_ClearValue.Color[3] = 0;
	}

	HZBBuffer::~HZBBuffer()
	{
		ReleaseResources();
	}

	void HZBBuffer::Init()
	{
		
	}

	void HZBBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		ReleaseResources();

		const IntPoint SizeClampedToPow2(Math::RoundDownToPowerOfTwo(Size.X), Math::RoundDownToPowerOfTwo(Size.Y));
		m_FirstMipSize = SizeClampedToPow2 / 2;
		m_FirstMipSize = IntPoint::ComponentWiseMax(m_FirstMipSize, IntPoint(1));

		int32 MinAxis = std::min(m_FirstMipSize.X, m_FirstMipSize.Y);
		m_MipCount = 1 + std::log2(MinAxis);

		M_HZBTarget = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(HZB_FORMAT, m_FirstMipSize.X, m_FirstMipSize.Y, 1, m_MipCount, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
		Desc.Format = HZB_FORMAT;
		Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		Desc.Texture2D.MipSlice = 0;
		Desc.Texture2D.PlaneSlice = 0;

		Device->CreateUnorderedAccessView(M_HZBTarget->GetD3D12Resource(), nullptr, &Desc, M_HZBTarget->GetCpuHandle());
	}

	void HZBBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void HZBBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void HZBBuffer::ReleaseResources()
	{
		if (M_HZBTarget)
		{
			M_HZBTarget->ReleaseBufferedResource();
			M_HZBTarget = nullptr;
		}
	}

}