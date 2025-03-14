#include "DrnPCH.h"
#include "D3D12Device.h"
#include "D3D12Adapter.h"

D3D12Device::D3D12Device(D3D12Adapter* InAdapter)
	: Adapter(InAdapter)
{
	
}

void D3D12Device::InitializeDevice()
{
	D3D12CreateDevice(Adapter->GetAdapter(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(Device.GetAddressOf()));
}