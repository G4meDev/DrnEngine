#pragma once

#include "ForwardTypes.h"

#include "taskflow.hpp"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#include "D3D12Utils.h"

LOG_DECLARE_CATEGORY( LogRenderer );

class IDXGISwapChain4;

// TODO: remove
#include <imgui.h>

namespace Drn
{
	class D3D12Scene;
	class Window;

	struct TempDescriptorHeapAllocator
	{
		ID3D12DescriptorHeap*       Heap     = nullptr;
		D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
		UINT                        HeapHandleIncrement;
		ImVector<int>               FreeIndices;

		void Create( ID3D12Device* device, ID3D12DescriptorHeap* heap )
		{
			Heap                            = heap;
			D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
			HeapType                        = desc.Type;
			HeapStartCpu                    = Heap->GetCPUDescriptorHandleForHeapStart();
			HeapStartGpu                    = Heap->GetGPUDescriptorHandleForHeapStart();
			HeapHandleIncrement             = device->GetDescriptorHandleIncrementSize( HeapType );
			FreeIndices.reserve( (int)desc.NumDescriptors );
			for ( int n = desc.NumDescriptors; n > 0; n-- )
				//FreeIndices.push_back( n - 1 );
				FreeIndices.push_front( n - 1 );
		}
		void Destroy()
		{
			Heap = nullptr;
			FreeIndices.clear();
		}
		void Alloc( D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle,
					D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle )
		{
			int idx = FreeIndices.back();
			FreeIndices.pop_back();
			out_cpu_desc_handle->ptr = HeapStartCpu.ptr + ( idx * HeapHandleIncrement );
			out_gpu_desc_handle->ptr = HeapStartGpu.ptr + ( idx * HeapHandleIncrement );
		}
		void Free( D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle,
					D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle )
		{
			int cpu_idx = (int)( ( out_cpu_desc_handle.ptr - HeapStartCpu.ptr ) / HeapHandleIncrement );
			int gpu_idx = (int)( ( out_gpu_desc_handle.ptr - HeapStartGpu.ptr ) / HeapHandleIncrement );
			if (cpu_idx == gpu_idx)
				//FreeIndices.push_back( cpu_idx );
				FreeIndices.push_front( cpu_idx );
		}
	};

	class Renderer
	{
	public:
		
		Renderer();
		virtual ~Renderer();


		static void Init(HINSTANCE inhInstance, Window* InMainWindow);
		static void Shutdown();

		void MainWindowResized(const IntPoint& NewSize);

		static Renderer* Get();

		inline Device* GetDevice() { return m_Device.get(); }
		inline ID3D12Device* GetD3D12Device() { return m_Device->GetD3D12Device(); }
		inline ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue.Get(); }

		inline Window* GetMainWindow() { return m_MainWindow; }

		void Tick(float DeltaTime);

		Scene* AllocateScene(World* InWorld);
		void ReleaseScene(Scene*& InScene);

		void SetHeaps( ID3D12GraphicsCommandList* CommandList);

		// TODO: delete
		Microsoft::WRL::ComPtr<ID3D12Fence>& GetFence() { return m_Fence; };
		uint64& GetFenceValue() { return m_FenceValue; };

		uint64_t m_FenceValue = 0;
		//uint64_t m_FrameFenceValues[NUM_BACKBUFFERS] = {};
		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		HANDLE m_FenceEvent;

		std::unique_ptr<Device> m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator[NUM_BACKBUFFERS];
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		//uint32 m_CurrentBackbufferIndex;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
		//uint32 m_RTVDescriporSize;
		//Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[NUM_BACKBUFFERS];

		SceneRenderer* m_MainSceneRenderer;

		inline ID3D12GraphicsCommandList2* GetCommandList() { return m_CommandList.Get(); };

		void Flush( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent );

		void Flush();

		void WaitForOnFlightCommands();

		static void ReportLiveObjects();

		inline void ToggleVSync() const { if(m_SwapChain) m_SwapChain->ToggleVSync(); }

		uint64_t Signal( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue );

		inline uint32 GetRtvIncrementSize() const { return m_RtvIncrementSize; }

		TempDescriptorHeapAllocator TempSRVAllocator;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvHeap;

		TempDescriptorHeapAllocator TempSamplerAllocator;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SamplerHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE LinearSamplerCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE LinearSamplerGpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE SamplerCpuHandle;

		tf::Taskflow m_RendererTickTask;
		tf::Taskflow m_DummyTask;

	protected:
		static Renderer* SingletonInstance;

		Window* m_MainWindow = nullptr;
		std::set<Scene*> m_AllocatedScenes;

		void WaitForFenceValue( Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent );

		uint32 m_RtvIncrementSize;

		friend class ViewportGuiLayer;
		friend class World;

	private:
		
		void Init_Internal();


	};
}