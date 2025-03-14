#include "DrnPCH.h"
#include "Renderer.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"

namespace Drn
{
	Renderer* Renderer::SingletonInstance;

	void Renderer::MakeInstance()
	{
		SingletonInstance = new Renderer();
	}

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init()
	{
		std::cout << "Renderer start!" << std::endl;

		bool bDebugDevice = false;

#if DRN_DEBUG
		bDebugDevice = true;
#endif

		Adapter = new D3D12Adapter();
		Adapter->InitalizeAdapter(bDebugDevice);

		MainDevice = new D3D12Device(Adapter);
		MainDevice->InitializeDevice();


	}

	void Renderer::Shutdown()
	{
		std::cout << "Renderer shutdown!" << std::endl;


	}

	void Renderer::Tick(float DeltaTime)
	{

	}

}