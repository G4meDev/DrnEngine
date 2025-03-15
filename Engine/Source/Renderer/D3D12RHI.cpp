#include "DrnPCH.h"
#include "D3D12RHI.h"
#include "D3D12Device.h"

namespace Drn
{
	Drn::D3D12RHI* D3D12RHI::SingletonInstance;
	
	void D3D12RHI::Init()
	{
		SingletonInstance = new D3D12RHI();
	}

	void D3D12RHI::Shutdown()
	{
		delete SingletonInstance;
	}

	D3D12RHI* D3D12RHI::Get()
	{
		return SingletonInstance;
	}

	void D3D12RHI::CreateCommandQueue(D3D12Device* Device, const D3D12_COMMAND_QUEUE_DESC& Desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>& OutCommandQueue)
	{
		VERIFYD3D12RESULT(Device->GetDevice()->CreateCommandQueue(&Desc, IID_PPV_ARGS(OutCommandQueue.GetAddressOf())));
	}


}