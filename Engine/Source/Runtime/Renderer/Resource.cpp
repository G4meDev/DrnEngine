#include "DrnPCH.h"
#include "Resource.h"

namespace Drn
{
	Resource::Resource()
		: BufferedResource()
	{
		Renderer::Get()->m_BindlessSrvHeapAllocator.Alloc(&m_CpuHandle, &m_GpuHandle);
	}

	Resource::~Resource()
	{
		Renderer::Get()->m_BindlessSrvHeapAllocator.Free(m_CpuHandle, m_GpuHandle);
	}

	Resource* Resource::Create( D3D12_HEAP_TYPE HeapType, const D3D12_RESOURCE_DESC& ResourceDescription,
		D3D12_RESOURCE_STATES InitalState)
	{
		CD3DX12_HEAP_PROPERTIES HeapPropperty( HeapType );

		Resource* Result = new Resource();
		Renderer::Get()->GetD3D12Device()->CreateCommittedResource( &HeapPropperty,
			D3D12_HEAP_FLAG_NONE, &ResourceDescription, InitalState, nullptr, IID_PPV_ARGS(Result->m_Resource.GetAddressOf()));

#if D3D12_Debug_INFO
		Result->SetName("UnnamedResource");
#endif

		return Result;
	}

	Resource* Resource::Create( D3D12_HEAP_TYPE HeapType, const D3D12_RESOURCE_DESC& ResourceDescription,
		D3D12_RESOURCE_STATES InitalState, const D3D12_CLEAR_VALUE& ClearValue)
	{
		Resource* Result = new Resource();

		CD3DX12_HEAP_PROPERTIES HeapPropperty( HeapType );

		Renderer::Get()->GetD3D12Device()->CreateCommittedResource( &HeapPropperty,
			D3D12_HEAP_FLAG_NONE, &ResourceDescription, InitalState, &ClearValue, IID_PPV_ARGS(Result->m_Resource.GetAddressOf()));

#if D3D12_Debug_INFO
		Result->SetName("UnnamedResource");
#endif

		return Result;
	}

#if D3D12_Debug_INFO
	void Resource::SetName( const std::string& Name )
	{
		if (m_Resource)
		{
			m_Resource->SetName(StringHelper::s2ws(Name).c_str());
		}
	}
#endif
}