#include "DrnPCH.h"
#include "ImGuiRenderer.h"

#if WITH_EDITOR

#include "ImGuiLayer.h"
#include "Runtime/Core/Window.h"
#include "Runtime/Renderer/Renderer.h"

#include <ImGuizmo.h>

LOG_DEFINE_CATEGORY( LogImguiRenderer, "ImguiRenderer" );

namespace Drn
{
	std::unique_ptr<ImGuiRenderer> ImGuiRenderer::SingletonInstance = nullptr;

	ImGuiRenderer::ImGuiRenderer()
	{

	}

	ImGuiRenderer::~ImGuiRenderer()
	{

	}

	void ImGuiRenderer::Init( class Window* MainWindow )
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplWin32_Init(MainWindow->GetWindowHandle());

		ImGui_ImplDX12_InitInfo init_info = {};

		init_info.Device = Renderer::Get()->GetD3D12Device();
		init_info.CommandQueue         = Renderer::Get()->GetCommandQueue();
		init_info.NumFramesInFlight = NUM_BACKBUFFERS;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

		init_info.SrvDescriptorHeap    = Renderer::Get()->m_BindlessSrvHeap.Get();
		init_info.SrvDescriptorAllocFn = []( ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE*out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle )
			{ return Renderer::Get()->m_BindlessSrvHeapAllocator.Alloc(out_cpu_handle, out_gpu_handle ); };
		 init_info.SrvDescriptorFreeFn = []( ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle )
			{ 	return Renderer::Get()->m_BindlessSrvHeapAllocator.Free( cpu_handle, gpu_handle ); };

		ImGui_ImplDX12_Init( &init_info );
	}

	void ImGuiRenderer::Tick( float DeltaTime, D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL)
	{
		SCOPE_STAT();

		PIXBeginEvent( CL, 1, "Imgui" );

		m_CommandList = CL;

		BeginDraw();
		Draw(DeltaTime);
		EndDraw( SwapChainCpuhandle, CL);

		PIXEndEvent( CL );
	}

	void ImGuiRenderer::Shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		for (auto it = Layers.begin(); it != Layers.end(); )
		{
			delete *it;
			it++;
		}
	}

	void ImGuiRenderer::AttachLayer( ImGuiLayer* InLayer )
	{
		Layers.insert(InLayer);
	}

	void ImGuiRenderer::DetachLayer( ImGuiLayer* InLayer ) 
	{
		Layers.erase(InLayer);
	}

	ImGuiRenderer* ImGuiRenderer::Get()
	{
		if (!SingletonInstance.get())
		{
			SingletonInstance = std::make_unique<ImGuiRenderer>();
		}

		return SingletonInstance.get();
	}

	void ImGuiRenderer::BeginDraw()
	{
		SCOPE_STAT();

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiRenderer::Draw( float DeltaTime )
	{
		SCOPE_STAT();

		for ( auto it = Layers.begin(); it != Layers.end(); )
		{
			if ((*it)->m_Open == false)
			{
				ImGuiLayer* Layer = *it; 
				it = Layers.erase(it);
				delete Layer;
			}
			else
			{
				++it;
			}
		}

		ImGui::DockSpaceOverViewport();
		
		for (auto it = Layers.begin(); it != Layers.end(); ++it)
		{
			(*it)->Draw(DeltaTime);
		}
	}

	void ImGuiRenderer::EndDraw( D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL )
	{
		SCOPE_STAT();

		ImGui::Render();
		Renderer::Get()->SetBindlessHeaps(CL);

		ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), CL );
	}

	void ImGuiRenderer::PostExecuteCommands()
	{
		SCOPE_STAT();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}

#endif