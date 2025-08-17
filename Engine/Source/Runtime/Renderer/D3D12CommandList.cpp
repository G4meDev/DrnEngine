#include "DrnPCH.h"
#include "D3D12CommandList.h"

namespace Drn
{
	D3D12CommandList::D3D12CommandList( ID3D12Device* Device, D3D12_COMMAND_LIST_TYPE Type, uint8 NumAllocators, const std::string& Name)
		: BufferedResource()
		, m_Type(Type)
		, m_NumAllocators(NumAllocators)
		, m_CurrentAllocatorIndex(0)
	{
		m_CommandAllocators.resize(NumAllocators);

		for (uint8 i = 0; i < NumAllocators; i++)
		{
			Device->CreateCommandAllocator(Type, IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf()));

#if D3D12_Debug_INFO
			m_CommandAllocators[i]->SetName( StringHelper::s2ws( "CommandAllocator_" + Name + "_" + std::to_string(i)).c_str() );
#endif
		}

		Device->CreateCommandList(NULL, Type, m_CommandAllocators[0].Get(), NULL, IID_PPV_ARGS(m_CommandList.GetAddressOf()));

#if D3D12_Debug_INFO
		m_CommandList->SetName( StringHelper::s2ws("CommandList_" + Name).c_str() );
#endif
	}

	D3D12CommandList::~D3D12CommandList()
	{
		
	}

	void D3D12CommandList::Close()
	{
		m_CommandList->Close();
	}

	void D3D12CommandList::FlipAndReset()
	{
		m_CurrentAllocatorIndex = (m_CurrentAllocatorIndex + 1) % m_NumAllocators;
		auto commandAllocator = m_CommandAllocators[m_CurrentAllocatorIndex];
		commandAllocator->Reset();
		m_CommandList->Reset(commandAllocator.Get(), nullptr);
	}

	void D3D12CommandList::SetAllocatorAndReset( uint8 AllocatorIndex )
	{
		m_CurrentAllocatorIndex = AllocatorIndex;
		auto commandAllocator = m_CommandAllocators[m_CurrentAllocatorIndex];
		commandAllocator->Reset();
		m_CommandList->Reset(commandAllocator.Get(), nullptr);
	}

}