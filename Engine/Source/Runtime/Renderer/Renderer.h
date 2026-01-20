#pragma once

#include "ForwardTypes.h"

#include "taskflow.hpp"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#include "D3D12Utils.h"
#include "Runtime/Renderer/GpuFence.h"

LOG_DECLARE_CATEGORY( LogRenderer );

class IDXGISwapChain4;

// TODO: remove
#include <imgui.h>

namespace Drn
{
	class D3D12Scene;
	class Window;
	class SamplerState;

	struct StaticSamplers
	{
	public:
		StaticSamplers() = default;

		uint32 LinearSampler;
		uint32 PointSampler;
		uint32 LinearCompLessSampler;
		uint32 LinearClampSampler;

		uint32 PointClampSampler;
		Vector Padding;
	};

	class Renderer
	{
	public:
		
		Renderer();
		virtual ~Renderer();

		static void Init(HINSTANCE inhInstance, Window* InMainWindow);
		static void Shutdown();

		inline uint64 GetFrameCount() const { return FrameCount; }

		void MainWindowResized(const IntPoint& NewSize);
		void SetFullScreen( bool bFullScreen);
		void ToggleFullScreen();

		static Renderer* Get();

		inline Device* GetDevice() { return m_Device.get(); }
		inline ID3D12Device* GetD3D12Device() { return m_Device->GetD3D12Device(); }
		inline ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue.Get(); }
		inline D3D12CommandList* GetCommandList() { return m_CommandList; }

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

		inline uint32 GetCurrentBackbufferIndex() const { return m_SwapChain ? m_SwapChain->GetBackBufferIndex() : 0; }

		inline GpuFence* GetFence() { return m_Fence; }
		inline GpuFence* GetDeletionFence() { return m_DeletionFence; }

		bool GetSwapchainContainingRect(RECT& OutRect);

		std::unique_ptr<Device> m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;

		TRefCountPtr<D3D12CommandList> m_CommandList;
		//TRefCountPtr<D3D12CommandList> m_UploadCommandList;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		SceneRenderer* m_MainSceneRenderer;

		void Flush();

		static void ReportLiveObjects();

		inline void ToggleVSync() const { if(m_SwapChain) m_SwapChain->ToggleVSync(); }

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_BindlessRootSinature;

		TRefCountPtr<SamplerState> LinearSampler;
		TRefCountPtr<SamplerState> PointSampler;

		TRefCountPtr<SamplerState> LinearCompLessSampler;

		TRefCountPtr<SamplerState> LinearClampSampler;
		TRefCountPtr<SamplerState> PointClampSampler;

		StaticSamplers m_StaticSamplers;
		TRefCountPtr<class RenderUniformBuffer> StaticSamplersBuffer;

		tf::Taskflow m_RendererTickTask;

		inline uint32 ComputeAnisotropy( uint32 InAnisotropy ) { return std::clamp(InAnisotropy > 0 ? InAnisotropy : MaxAnisotropy, 1u, 16u);}

	protected:
		static Renderer* SingletonInstance;

		Window* m_MainWindow = nullptr;
		std::set<Scene*> m_AllocatedScenes;

		friend class ViewportGuiLayer;
		friend class World;

	private:
		
		void Init_Internal();

		TRefCountPtr<class GpuFence> m_Fence;
		TRefCountPtr<class GpuFence> m_DeletionFence;
		uint64 FrameCount = 0;
		uint32 MaxAnisotropy = 16;

#if WITH_EDITOR

	public:
		void StartGpuFrameCapture();
		void EndGpuFrameCapture();

		void MarkFrameForCapture();

	private:
		struct RENDERDOC_API_1_6_0* rdoc_api = NULL;
		bool bCapturingFrame = false;
#endif
	};
}