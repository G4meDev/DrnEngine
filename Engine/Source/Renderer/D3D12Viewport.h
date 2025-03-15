#pragma once

#include "imgui.h"

#define NUM_BACKBUFFERS 3

namespace Drn
{
	class D3D12Adapter;
	class D3D12Texture;
	class D3D12Queue;
	class D3D12DescriptorHeap;
	class D3D12CommandAllocator;

	struct ExampleDescriptorHeapAllocator
	{
		ID3D12DescriptorHeap* Heap = nullptr;
		D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
		UINT                        HeapHandleIncrement;
		ImVector<int>               FreeIndices;

		void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
		{
			IM_ASSERT(Heap == nullptr && FreeIndices.empty());
			Heap = heap;
			D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
			HeapType = desc.Type;
			HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
			HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
			HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
			FreeIndices.reserve((int)desc.NumDescriptors);
			for (int n = desc.NumDescriptors; n > 0; n--)
				FreeIndices.push_back(n - 1);
		}
		void Destroy()
		{
			Heap = nullptr;
			FreeIndices.clear();
		}
		void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
		{
			IM_ASSERT(FreeIndices.Size > 0);
			int idx = FreeIndices.back();
			FreeIndices.pop_back();
			out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
			out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
		}
		void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
		{
			int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
			int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
			IM_ASSERT(cpu_idx == gpu_idx);
			FreeIndices.push_back(cpu_idx);
		}
	};

	class D3D12Viewport
	{
	public:

		D3D12Viewport(D3D12Adapter* InAdapter, HWND InWindowHandle, UINT InSizeX, UINT InSizeY, bool InFullScreen, DXGI_FORMAT InPixelFormat);

		void Init();

		void Tick(float DeltaTime);

		inline D3D12Queue* GetQueue_Direct() { return CommandQueue_Direct; }
		inline D3D12Queue* GetQueue_Copy() { return CommandQueue_Copy; }
		inline D3D12Queue* GetQueue_Compute() { return CommandQueue_Compute; }


	protected:

		Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;

		D3D12Adapter* Adapter;
		HWND WindowHandle;
		UINT SizeX;
		UINT SizeY;
		DXGI_FORMAT PixelFormat;
		bool bFullScreen;

		UINT BackBufferIndex;

		CD3DX12_VIEWPORT Viewport;
		CD3DX12_RECT ScissorRect;

	private:
		
		static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;

		Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
		ULONG FenceValue;
		HANDLE FenceEvent;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

		std::shared_ptr<D3D12CommandAllocator> CommandAllocator_Direct;

		//std::shared_ptr<D3D12Texture> BackBuffers[NUM_BACKBUFFERS];
		std::shared_ptr<ID3D12Resource> BackBuffers[NUM_BACKBUFFERS];

		D3D12DescriptorHeap* SwapChainDescriptorRVTRoot;
		std::shared_ptr<D3D12DescriptorHeap> SwapChainDescriptorRVT[NUM_BACKBUFFERS];

		std::shared_ptr<D3D12DescriptorHeap> DescriptorHeapSRV;

		D3D12Queue* CommandQueue_Direct;
		D3D12Queue* CommandQueue_Copy;
		D3D12Queue* CommandQueue_Compute;

		std::shared_ptr<D3D12CommandAllocator> CommandAllocators_Direct[NUM_BACKBUFFERS];
		std::shared_ptr<D3D12CommandAllocator> CommandAllocators_Copy[NUM_BACKBUFFERS];
		std::shared_ptr<D3D12CommandAllocator> CommandAllocators_Compute[NUM_BACKBUFFERS];


		void WaitForPreviousFrame();
	};
}