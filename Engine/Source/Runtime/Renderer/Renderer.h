#pragma once

#include "ForwardTypes.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>


LOG_DECLARE_CATEGORY( LogRenderer );

namespace Drn
{
	class D3D12Scene;

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
		void RemoveScene(Scene* InScene);
		void RemoveAndInvalidateScene(Scene*& InScene);

		float TotalTime = 0;

		std::shared_ptr<dx12lib::CommandList> m_CommandList;

	protected:
		static Renderer* SingletonInstance;

		Window* m_MainWindow = nullptr;
		
		std::shared_ptr<::dx12lib::Device> m_Device = nullptr;

		std::shared_ptr<::dx12lib::SwapChain> m_SwapChain = nullptr;

		std::shared_ptr<dx12lib::VertexBuffer> m_VertexBuffer = nullptr;
		std::shared_ptr<dx12lib::IndexBuffer>  m_IndexBuffer  = nullptr;

		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature       = nullptr;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject = nullptr;

		std::shared_ptr<dx12lib::Texture> m_DepthTexture = nullptr;

		dx12lib::RenderTarget m_RenderTarget;

		float m_fieldOfView = 45.0f;

		World* MainWorld;
		Scene* MainScene;
		SceneRenderer* MainSceneRenderer;

		StaticMeshComponent* CubeMeshComponent;
		CameraComponent* Camera;
		float CameraSpeed = 0.01f;

		std::set<Scene*> AllocatedScenes;

		friend class ViewportGuiLayer;

	private:
		
		void Init_Internal();
	};
}