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
	{}

	HZBBuffer::~HZBBuffer()
	{}

	void HZBBuffer::Init()
	{}

	void HZBBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		const IntPoint SizeClampedToPow2(Math::RoundDownToPowerOfTwo(Size.X), Math::RoundDownToPowerOfTwo(Size.Y));
		m_FirstMipSize = SizeClampedToPow2 / 2;
		m_FirstMipSize = IntPoint::ComponentWiseMax(m_FirstMipSize, IntPoint(1));

		int32 MinAxis = std::min(m_FirstMipSize.X, m_FirstMipSize.Y);
		m_MipCount = 1 + std::log2(MinAxis);

		RenderResourceCreateInfo HzbTargetCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "Hzb_Target" );
		M_HZBTarget = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), m_FirstMipSize.X, m_FirstMipSize.Y, HZB_FORMAT, m_MipCount, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::UAV | ETextureCreateFlags::ShaderResource), HzbTargetCreateInfo);

		m_UAVHandles.clear();
		const int32 UavCount = m_MipCount;
		for (int32 i = 0; i < UavCount; i++)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
			Desc.Format = HZB_FORMAT;
			Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			Desc.Texture2D.MipSlice = i;
			Desc.Texture2D.PlaneSlice = 0;

			m_UAVHandles.push_back(new UnorderedAccessView(Renderer::Get()->GetDevice(), Desc, M_HZBTarget->m_ResourceLocation));
		}

		m_SRVHandles.clear();
		const int32 SrvCount = m_MipCount / 4;
		for (int32 i = 0; i < SrvCount; i++)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
			Desc.Format = HZB_FORMAT;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.Texture2D.MipLevels = 1;
			Desc.Texture2D.MostDetailedMip = i * 4 + 3;

			m_SRVHandles.push_back(new ShaderResourceView(Renderer::Get()->GetDevice(), Desc, M_HZBTarget->m_ResourceLocation));
		}

		//{
		//	D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
		//	Desc.Format = HZB_FORMAT;
		//	Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//	Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//	Desc.Texture2D.MipLevels = m_MipCount;
		//	Desc.Texture2D.MostDetailedMip = 0;
		//	
		//	Device->CreateShaderResourceView(M_HZBTarget->GetD3D12Resource(), &Desc, M_HZBTarget->GetCpuHandle());
		//}
		//
		//for (int32 i = 0; i < m_UAVHandles.size(); i++)
		//{
		//	D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
		//	Desc.Format = HZB_FORMAT;
		//	Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		//	Desc.Texture2D.MipSlice = i;
		//	Desc.Texture2D.PlaneSlice = 0;
		//	
		//	Device->CreateUnorderedAccessView(M_HZBTarget->GetD3D12Resource(), nullptr, &Desc, m_UAVHandles[i].CpuHandle);
		//}
		//
		//for (int32 i = 0; i < m_SrvHandles.size(); i++)
		//{
		//	D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
		//	Desc.Format = HZB_FORMAT;
		//	Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//	Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//	Desc.Texture2D.MipLevels = 1;
		//	Desc.Texture2D.MostDetailedMip = i * 4 + 3;
		//
		//	Device->CreateShaderResourceView(M_HZBTarget->GetD3D12Resource(), &Desc, m_SrvHandles[i].CpuHandle);
		//}

	}

	void HZBBuffer::Clear( D3D12CommandList* CommandList )
	{
		
	}

	void HZBBuffer::Bind( D3D12CommandList* CommandList )
	{
		
	}

	void HZBBuffer::ReallocateViewHandles()
	{
		//auto ReallocateHandles = [&](std::vector<HZBViewHandle>& Views, int32 NewSize)
		//{
		//	int32 Diff = NewSize - Views.size();
		//
		//	if ( Diff > 0 )
		//	{
		//		Views.resize(NewSize);
		//
		//		for ( int32 i = NewSize - Diff; i < NewSize; i++ )
		//		{
		//			Renderer::Get()->m_BindlessSrvHeapAllocator.Alloc(&Views[i].CpuHandle, &Views[i].GpuHandle);
		//		}
		//	}
		//
		//	else if ( Diff < 0 )
		//	{
		//		for (int32 i = Views.size() + Diff; i < Views.size(); i++)
		//		{
		//			Renderer::Get()->m_BindlessSrvHeapAllocator.Free(Views[i].CpuHandle, Views[i].GpuHandle);
		//		}
		//
		//		Views.resize(NewSize);
		//	}
		//};
		//
		//const int32 UAVSize = m_MipCount;
		//const int32 SrvSize = m_MipCount / 4; //there is only need for one view for each 4 mip.
		//
		//ReallocateHandles(m_UAVHandles, UAVSize);
		//ReallocateHandles(m_SrvHandles, SrvSize);
	}
}