#include "DrnPCH.h"
#include "D3D12Device.h"
#include "D3D12Adapter.h"
#include "D3D12Queue.h"

namespace Drn
{
	D3D12Device::D3D12Device(D3D12Adapter* InAdapter)
		: Adapter(InAdapter)
	{
	}

	void D3D12Device::Initialize()
	{
		CommandQueue_Direct = new D3D12Queue(this, D3D12QueueType::Direct);
		CommandQueue_Copy = new D3D12Queue(this, D3D12QueueType::Copy);
		CommandQueue_Compute = new D3D12Queue(this, D3D12QueueType::Compute);
	}

}