#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

LOG_DECLARE_CATEGORY(LogImguiRenderer);

namespace Drn
{
	class ImGuiLayer;

	struct ExampleDescriptorHeapAllocator
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
					FreeIndices.push_back( n - 1 );
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
				FreeIndices.push_back( cpu_idx );
			}
		};

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer();

		virtual void Init();
		virtual void Tick(float DeltaTime, D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL);
		virtual void Shutdown();

		void AttachLayer(ImGuiLayer* InLayer);
		void DetachLayer(ImGuiLayer* InLayer);

		static ImGuiRenderer* Get();
		virtual void PostExecuteCommands();

		static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;
	
	protected:
		virtual void BeginDraw();
		virtual void Draw();
		virtual void EndDraw( D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL );

		friend class Renderer;

		ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;

	private:
		static std::unique_ptr<ImGuiRenderer> SingletonInstance;
		std::set<ImGuiLayer*> Layers;
		//LinkedList<ImGuiLayer> Layers;
		std::vector<ImGuiLayer*> m_CurrentTickLayers;
	
	};
}
#endif