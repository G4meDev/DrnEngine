#pragma once

#include "ForwardTypes.h"

#include "D3D12Utils.h"
#include "d3dx12.h"

namespace Drn
{
	class SwapChain
	{
	public:
		SwapChain(Device* InDevice, HWND WindowHandle, ID3D12CommandQueue* CommandQueue, const IntPoint& Size);
		~SwapChain();

		inline uint32 GetBackBufferIndex() const { return m_CurrentBackbufferIndex; }
		inline ID3D12Resource* GetBackBuffer() { return m_BackBuffers[m_CurrentBackbufferIndex].Get(); }

		void Present();

		void Resize(const IntPoint& NewSize);

		inline void ToggleVSync() { m_Vsync = !m_Vsync; }
		bool CheckTearingSupport();

		inline uint64 GetFenceValue() { return m_FrameFenceValues[m_CurrentBackbufferIndex]; }

		inline D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferHandle()
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE( m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				m_CurrentBackbufferIndex, m_RTVDescriporSize );
		}

	private:

		void UpdateRenderTargetViews();

		Device* m_Device;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
		ID3D12CommandQueue* m_CommandQueue;

		uint64_t m_FrameFenceValues[NUM_BACKBUFFERS] = {};
		Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[NUM_BACKBUFFERS];
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
		uint32 m_RTVDescriporSize;
		uint32 m_CurrentBackbufferIndex;

		IntPoint m_Size;
		bool m_TearingSupported;
		bool m_Vsync = true;

		friend class Renderer;
	};
}