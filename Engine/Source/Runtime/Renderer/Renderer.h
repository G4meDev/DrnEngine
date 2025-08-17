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

	struct StaticSamplers
	{
	public:
		StaticSamplers() = default;

		uint32 LinearSampler;
		uint32 PointSampler;
		uint32 LinearCompLessSampler;
		uint32 LinearClampSampler;
		uint32 PointClampSampler;
	};

	struct TempDescriptorHeapAllocator
	{
		ID3D12DescriptorHeap*       Heap     = nullptr;
		D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
		UINT                        HeapHandleIncrement;
		std::vector<uint32> FreeIndices;

		void Create( ID3D12Device* device, ID3D12DescriptorHeap* heap )
		{
			Heap                            = heap;
			D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
			HeapType                        = desc.Type;
			HeapStartCpu                    = Heap->GetCPUDescriptorHandleForHeapStart();
			HeapStartGpu                    = Heap->GetGPUDescriptorHandleForHeapStart();
			HeapHandleIncrement             = device->GetDescriptorHandleIncrementSize( HeapType );
			FreeIndices.reserve( (int)desc.NumDescriptors );
			for ( int n = 0; n < desc.NumDescriptors; n++ )
				FreeIndices.push_back(n);
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
				FreeIndices.push_back( cpu_idx );
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
		void InitRender(float DeltaTime);
		void UpdateSceneProxyAndResources();
		void RenderSceneRenderers();
		void RenderImgui();
		void ResolveDisplayBuffer();
		void ExecuteCommands();

		Scene* AllocateScene(World* InWorld);
		void ReleaseScene(Scene*& InScene);

		void SetBindlessHeaps( ID3D12GraphicsCommandList* CommandList);

		// TODO: delete
		Microsoft::WRL::ComPtr<ID3D12Fence>& GetFence() { return m_Fence; };
		uint64& GetFenceValue() { return m_FenceValue; };

		uint64_t m_FenceValue = 0;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		HANDLE m_FenceEvent;

		std::unique_ptr<Device> m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;

		D3D12CommandList* m_CommandList;
		D3D12CommandList* m_UploadCommandList;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		SceneRenderer* m_MainSceneRenderer;

		ID3D12GraphicsCommandList2* GetCommandList();

		void Flush();

		static void ReportLiveObjects();

		inline void ToggleVSync() const { if(m_SwapChain) m_SwapChain->ToggleVSync(); }

		uint64_t Signal( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue );

		inline uint32 GetRtvIncrementSize() const { return m_RtvIncrementSize; }
		inline uint32 GetDsvIncrementSize() const { return m_DsvIncrementSize; }
		inline uint32 GetSrvIncrementSize() const { return m_SrvIncrementSize; }

		// TODO: remove
		TempDescriptorHeapAllocator m_BindlessSrvHeapAllocator;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_BindlessSrvHeap;

		TempDescriptorHeapAllocator m_BindlessSamplerHeapAllocator;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_BindlessSamplerHeap;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_BindlessRootSinature;

		D3D12_CPU_DESCRIPTOR_HANDLE m_BindlessLinearSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_BindlessLinearSamplerGpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE m_BindlessPointSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_BindlessPointSamplerGpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE m_BindlessLinearCompLessSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_BindlessLinearCompLessSamplerGpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE m_BindlessLinearClampSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_BindlessLinearClampSamplerGpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE m_BindlessPointClampSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_BindlessPointClampSamplerGpuHandle;

		StaticSamplers m_StaticSamplers;
		Resource* m_StaticSamplersBuffer;

		uint32 GetBindlessSrvIndex(D3D12_GPU_DESCRIPTOR_HANDLE Handle);
		uint32 GetBindlessSamplerIndex(D3D12_GPU_DESCRIPTOR_HANDLE Handle);

		tf::Taskflow m_RendererTickTask;

	protected:
		static Renderer* SingletonInstance;

		Window* m_MainWindow = nullptr;
		std::set<Scene*> m_AllocatedScenes;

		void WaitForFenceValue( Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent );

		uint32 m_RtvIncrementSize;
		uint32 m_DsvIncrementSize;
		uint32 m_SrvIncrementSize;
		uint32 m_SamplerIncrementSize;

		friend class ViewportGuiLayer;
		friend class World;

	private:
		
		void Init_Internal();


	};
}