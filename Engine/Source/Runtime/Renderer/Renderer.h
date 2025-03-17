#pragma once

#include "Runtime/Renderer/D3D12Queue.h"

namespace Drn
{
	class D3D12Adapter;
	class D3D12Device;
	class D3D12DescriptorHeap;

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

		void WaitForPreviousFrame();

	protected:
		static Renderer* SingletonInstance;

		std::unique_ptr<Window> MainWindow = nullptr;

		ID3D12GraphicsCommandList* CommandList;
		std::unique_ptr<D3D12CommandAllocator> CommandAllocator;
		std::unique_ptr<D3D12Queue> CommandQueue;


		Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;

		UINT BackBufferIndex;

		std::shared_ptr<ID3D12Resource> BackBuffers[NUM_BACKBUFFERS];

		D3D12DescriptorHeap* SwapChainDescriptorRVTRoot;
		std::shared_ptr<D3D12DescriptorHeap> SwapChainDescriptorRVT[NUM_BACKBUFFERS];

		Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
		ULONG FenceValue;
		HANDLE FenceEvent;

	private:
		
		void CreateResources();
		void CreateSwapChain();
		void ResetResources();
		void BindFrontBuffer();
		void ExecuteCommandList();
	};
}