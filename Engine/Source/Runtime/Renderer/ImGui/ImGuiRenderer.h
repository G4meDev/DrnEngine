#pragma once

//#if WITH_EDITOR

#include <GameFramework/GameFramework.h>
#include <GameFramework/Window.h>
#include <dx12lib/Device.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/CommandList.h>


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"


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

		virtual void Init( dx12lib::Device* InDevice, ID3D12Resource* InViewportResource);
		virtual void Tick(float DeltaTime, D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL);

		void AttachLayer(ImGuiLayer* InLayer);

		static ImGuiRenderer* Get();
		virtual void PostExecuteCommands();

        void OnViewportResize( float InWidth, float InHeight, ID3D12Resource* InView);

	protected:
		virtual void BeginDraw();
		virtual void Draw();
                virtual void EndDraw( D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL );
		
            void WaitForPreviousFrame();

        float Width;
        float Height;

		friend class Renderer;

		dx12lib::Device* m_Device;

        ID3D12Resource* ViewportResource;

		ID3D12DescriptorHeap*          g_pd3dSrvDescHeap = nullptr;
        static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;
        ID3D12CommandQueue*            g_pd3dCommandQueue = nullptr;
        ID3D12CommandAllocator*                CommandAllocator;
        ID3D12GraphicsCommandList*      g_pd3dCommandList = nullptr;

        D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;

        bool bInitalized = false;
        bool ViewportSizeDirty = false;

        UINT                m_frameIndex;
        HANDLE              m_fenceEvent;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        UINT64              m_fenceValue;


	private:
		static std::unique_ptr<ImGuiRenderer> SingletonInstance;
		LinkedList<ImGuiLayer> Layers;
	};
}
//#endif