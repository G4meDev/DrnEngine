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

	ID3D12Device* D3D12Device::GetD3DDevice()
	{
		return Adapter->GetD3DDevice();
	}

	void D3D12Device::Initialize()
	{

	}

}