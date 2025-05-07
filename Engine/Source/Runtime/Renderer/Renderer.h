#pragma once

#include "ForwardTypes.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#include "D3D12Utils.h"

LOG_DECLARE_CATEGORY( LogRenderer );

class IDXGISwapChain4;

namespace Drn
{
	class D3D12Scene;
	class Window;

	class Renderer
	{
	public:
		
		Renderer();
		virtual ~Renderer();


		static void Init(HINSTANCE inhInstance, Window* InMainWindow);
		static void Shutdown();

		void MainWindowResized(const IntPoint& NewSize);

		static Renderer* Get();

		inline ID3D12Device* GetD3D12Device() { return m_Device.Get(); }
		inline ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue.Get(); }

		inline Window* GetMainWindow() { return m_MainWindow; }

		void Tick(float DeltaTime);

		Scene* AllocateScene(World* InWorld);
		void ReleaseScene(Scene*& InScene);

		float TotalTime = 0;

		uint64_t m_FenceValue = 0;
		uint64_t m_FrameFenceValues[NUM_BACKBUFFERS] = {};
		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		HANDLE m_FenceEvent;

		Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator[NUM_BACKBUFFERS];
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
		uint32 m_RTVDescriporSize;
		uint32 m_CurrentBackbufferIndex;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[NUM_BACKBUFFERS];

		SceneRenderer* m_MainSceneRenderer;

		inline ID3D12GraphicsCommandList2* GetCommandList() { return m_CommandList.Get(); };

		void Flush( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent );

		void Flush();

		static void ReportLiveObjects();

		inline void ToggleVSync()
		{
			m_Vsync = !m_Vsync;
		}

	protected:
		static Renderer* SingletonInstance;

		Window* m_MainWindow = nullptr;
		std::set<Scene*> m_AllocatedScenes;

		void UpdateRenderTargetViews();

		uint64_t Signal( Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue );
		void WaitForFenceValue( Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent );

		friend class ViewportGuiLayer;
		friend class World;

	private:
		
		void Init_Internal();
		bool CheckTearingSupport();

		bool m_Vsync = true;
		bool m_TearingSupported = false;
	};
}