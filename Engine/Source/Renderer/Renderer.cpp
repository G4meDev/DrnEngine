#include "DrnPCH.h"
#include "Renderer.h"
#include "D3D12RHI.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"

namespace Drn
{
	Renderer* Renderer::SingletonInstance;

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	Drn::D3D12RHI* Renderer::GetRHI()
	{
		return D3D12RHI::Get();
	}

	void Renderer::Init()
	{
		std::cout << "Renderer start!" << std::endl;
		SingletonInstance = new Renderer();

		D3D12RHI::Init();

		Get()->Adapter = new D3D12Adapter();
		Get()->MainDevice = new D3D12Device(Get()->Adapter);
	}

	void Renderer::Shutdown()
	{
		std::cout << "Renderer shutdown!" << std::endl;
	}

	void Renderer::Tick(float DeltaTime)
	{

	}

}