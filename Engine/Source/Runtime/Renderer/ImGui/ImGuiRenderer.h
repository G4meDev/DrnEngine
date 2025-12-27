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
	class Window;
	class D3D12CommandList;

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer();

		virtual void Init(Window* MainWindow);
		virtual void Tick(float DeltaTime, D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, D3D12CommandList* CL);
		virtual void Shutdown();

		void AttachLayer(ImGuiLayer* InLayer);
		void DetachLayer(ImGuiLayer* InLayer);

		static ImGuiRenderer* Get();
		virtual void PostExecuteCommands();

		inline D3D12CommandList* GetCommandList() const { return m_CommandList; }

	protected:
		virtual void BeginDraw();
		virtual void Draw(float DeltaTime);
		virtual void EndDraw( D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, D3D12CommandList* CL );

		friend class Renderer;

	private:
		static std::unique_ptr<ImGuiRenderer> SingletonInstance;

		D3D12CommandList* m_CommandList;

		std::set<ImGuiLayer*> Layers;
		//LinkedList<ImGuiLayer> Layers;
		std::vector<ImGuiLayer*> m_CurrentTickLayers;
	
	};
}
#endif