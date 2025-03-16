#include "DrnPCH.h"
#include "D3D12Queue.h"
#include "Renderer.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"

namespace Drn
{

	D3D12Queue::D3D12Queue(D3D12Device* Device, D3D12QueueType QueueType)
		: Device(Device)
		, QueueType(QueueType)
	{
		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
		CommandQueueDesc.Type = GetD3DCommandListType(QueueType);
		CommandQueueDesc.Priority = 0;
		CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		Renderer::Get()->CreateCommandQueue(Device, CommandQueueDesc, CommandQueue);

		CommandQueue->SetName(L"Untitled Command Queue");

	}

	D3D12Queue::~D3D12Queue()
	{

	}

	D3D12CommandAllocator::D3D12CommandAllocator(D3D12Device* Device, D3D12QueueType QueueType)
		: Device(Device)
		, QueueType(QueueType)
	{
		Device->GetD3DDevice()->CreateCommandAllocator(GetD3DCommandListType(QueueType), IID_PPV_ARGS(CommandAllocator.GetAddressOf()));
	}

	D3D12CommandAllocator::~D3D12CommandAllocator()
	{

	}
}