#include "DrnPCH.h"
#include "D3D12Queue.h"
#include "Renderer.h"
#include "D3D12RHI.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"

namespace Drn
{

	D3D12Queue::D3D12Queue(D3D12Device* Device, D3D12QueueType QueueType)
		: Device(Device)
		, QueueType(QueueType)
	{
		D3D12Adapter* Adapter = Device->GetAdapter();

		D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
		CommandQueueDesc.Type = GetD3DCommandListType(QueueType);
		CommandQueueDesc.Priority = 0;
		CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

 		Renderer::GetRHI()->CreateCommandQueue(Device, CommandQueueDesc, CommandQueue);
		
 		CommandQueue->SetName(L"Untitled Command Queue");

	}

	D3D12Queue::~D3D12Queue()
	{

	}
}
