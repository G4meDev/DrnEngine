#include "DrnPCH.h"
#include "Renderer.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"

namespace Drn
{
	Renderer* Renderer::SingletonInstance;

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init()
	{
		std::cout << "Renderer start!" << std::endl;
		SingletonInstance = new Renderer();

		Get()->Adapter = new D3D12Adapter();
	}

	void Renderer::Shutdown()
	{
		std::cout << "Renderer shutdown!" << std::endl;
	}

	void Renderer::Tick(float DeltaTime)
	{

	}

	void Renderer::CreateCommandQueue(D3D12Device* Device, const D3D12_COMMAND_QUEUE_DESC& Desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>& OutCommandQueue)
	{
		VERIFYD3D12RESULT(Device->GetD3DDevice()->CreateCommandQueue(&Desc, IID_PPV_ARGS(OutCommandQueue.GetAddressOf())));
	}
}