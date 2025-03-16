#include "DrnPCH.h"
#include "Renderer.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"
#include "D3D12Viewport.h"
#include "D3D12Queue.h"

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	Renderer* Renderer::SingletonInstance;

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init(HINSTANCE inhInstance)
	{
		std::cout << "Renderer start!" << std::endl;
		SingletonInstance = new Renderer();

		Get()->Adapter = new D3D12Adapter();

		Get()->MainWindow = std::make_unique<Window>(inhInstance, Renderer::Get()->Adapter, std::wstring(L"Untitled window"));

		ImGuiRenderer::Get()->Init(Get()->GetMainWindow()->GetViewport()->GetQueue_Direct()->CommandQueue.Get(), Get()->GetMainWindow()->GetViewport()->GetCommandList(), Get()->GetMainWindow()->GetViewport()->GetOutputBuffer());
	}

	void Renderer::Shutdown()
	{
		std::cout << "Renderer shutdown!" << std::endl;
	}

	void Renderer::Tick(float DeltaTime)
	{
		MainWindow->Tick(DeltaTime);
	}

	void Renderer::CreateCommandQueue(D3D12Device* Device, const D3D12_COMMAND_QUEUE_DESC& Desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>& OutCommandQueue)
	{
		VERIFYD3D12RESULT(Device->GetD3DDevice()->CreateCommandQueue(&Desc, IID_PPV_ARGS(OutCommandQueue.GetAddressOf())));
	}
}