#include "DrnPCH.h"
#include "Resource.h"

namespace Drn
{
	Resource::Resource()
		: BufferedResource()
	{
	}

	Resource::~Resource()
	{
	}

	Resource* Resource::Create( D3D12_HEAP_TYPE HeapType, const D3D12_RESOURCE_DESC& ResourceDescription,
		D3D12_RESOURCE_STATES InitalState )
	{
		Resource* Result = new Resource();
		Renderer::Get()->GetD3D12Device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( HeapType ),
			D3D12_HEAP_FLAG_NONE, &ResourceDescription, InitalState, nullptr, IID_PPV_ARGS(Result->m_Resource.GetAddressOf()));

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