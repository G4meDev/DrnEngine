#pragma once

namespace Drn
{
	class D3D12Adapter;
	class D3D12Device;

	class Renderer
	{
	public:
		
		Renderer(){};

		static void Init(HINSTANCE inhInstance);
		static void Shutdown();

		static Renderer* Get();

		void Tick(float DeltaTime);

		D3D12Adapter* Adapter;

		inline Window* GetMainWindow() { return MainWindow.get(); }

		void CreateCommandQueue(D3D12Device* Device, const D3D12_COMMAND_QUEUE_DESC& Desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>& OutCommandQueue);

	protected:
		static Renderer* SingletonInstance;

		std::unique_ptr<Window> MainWindow = nullptr;

	private:

	};
}