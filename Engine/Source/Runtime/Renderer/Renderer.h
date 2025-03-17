#pragma once

#include "Runtime/Renderer/D3D12Queue.h"

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

		inline ID3D12GraphicsCommandList* GetCommandList() { return CommandList; }
		inline ID3D12CommandQueue* GetCommandQueue() { return CommandQueue->CommandQueue.Get(); }

		void CreateCommandQueue(D3D12Device* Device, const D3D12_COMMAND_QUEUE_DESC& Desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>& OutCommandQueue);

	protected:
		static Renderer* SingletonInstance;

		std::unique_ptr<Window> MainWindow = nullptr;

		ID3D12GraphicsCommandList* CommandList;
		std::unique_ptr<D3D12CommandAllocator> CommandAllocator;
		std::unique_ptr<D3D12Queue> CommandQueue;

	private:
		
		void CreateResources();

		void ResetResources();
	};
}