#pragma once

#include "ForwardTypes.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>


LOG_DECLARE_CATEGORY( LogRenderer );

namespace Drn
{
	class D3D12Scene;
	class Window;

	class Renderer
	{
	public:
		
		Renderer();

		static void Init(HINSTANCE inhInstance, Window* InMainWindow);
		static void Shutdown();

		void ToggleSwapChain();

		void MainWindowResized(float InWidth, float InHeight);

		static Renderer* Get();

		inline dx12lib::Device* GetDevice() { return m_Device.get(); }

		inline Window* GetMainWindow() { return m_MainWindow; }

		void Tick(float DeltaTime);

		Scene* AllocateScene(World* InWorld);
		void ReleaseScene(Scene*& InScene);

		float TotalTime = 0;

		std::shared_ptr<dx12lib::CommandList> m_CommandList;

		SceneRenderer* m_MainSceneRenderer;

		inline std::shared_ptr<dx12lib::CommandList> GetCommandList() { return m_CommandList; };


	protected:
		static Renderer* SingletonInstance;

		Window* m_MainWindow = nullptr;
		
		std::shared_ptr<::dx12lib::Device> m_Device = nullptr;

		std::shared_ptr<::dx12lib::SwapChain> m_SwapChain = nullptr;

		std::set<Scene*> m_AllocatedScenes;


		friend class ViewportGuiLayer;
		friend class World;

	private:
		
		void Init_Internal();
	};
}