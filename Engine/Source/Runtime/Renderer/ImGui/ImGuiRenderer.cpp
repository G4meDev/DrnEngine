#include "DrnPCH.h"
#include "ImGuiRenderer.h"

#if WITH_EDITOR

#include "ImGuiLayer.h"
#include "Runtime/Core/Window.h"
#include "Runtime/Renderer/Renderer.h"

LOG_DEFINE_CATEGORY( LogImguiRenderer, "ImguiRenderer" );

namespace Drn
{
	std::unique_ptr<ImGuiRenderer> ImGuiRenderer::SingletonInstance = nullptr;
	ExampleDescriptorHeapAllocator ImGuiRenderer::g_pd3dSrvDescHeapAlloc;

	ImGuiRenderer::ImGuiRenderer()
	{

	}

	ImGuiRenderer::~ImGuiRenderer()
	{

	}

	void ImGuiRenderer::Init( class Window* MainWindow )
	{
		ID3D12Device* pDevice = Renderer::Get()->GetDevice()->GetD3D12Device().Get();

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors             = 64;
		desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		pDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &g_pd3dSrvDescHeap ) ); 
		g_pd3dSrvDescHeapAlloc.Create( pDevice, g_pd3dSrvDescHeap );

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

		//auto Wind = GameFramework::Get().GetWindowByName( L"Clear Screen" );
		ImGui_ImplWin32_Init(MainWindow->GetWindowHandle());

		ImGui_ImplDX12_InitInfo init_info = {};

		init_info.Device = pDevice;
		init_info.CommandQueue         = Renderer::Get()->GetDevice()->GetCommandQueue().GetD3D12CommandQueue().Get();
		init_info.NumFramesInFlight = NUM_BACKBUFFERS;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		init_info.SrvDescriptorHeap    = g_pd3dSrvDescHeap;
		init_info.SrvDescriptorAllocFn = []( ImGui_ImplDX12_InitInfo*,
												D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
												D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle ) {
			return g_pd3dSrvDescHeapAlloc.Alloc( out_cpu_handle, out_gpu_handle );
		};
		init_info.SrvDescriptorFreeFn = []( ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
											D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle ) {
			return g_pd3dSrvDescHeapAlloc.Free( cpu_handle, gpu_handle );
		};
		ImGui_ImplDX12_Init( &init_info );
	}

	void ImGuiRenderer::Tick( float DeltaTime, D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL)
	{
		SCOPE_STAT(ImGuiRendererTick);

		BeginDraw();
		Draw(DeltaTime);
		EndDraw( SwapChainCpuhandle, CL);
	}

	void ImGuiRenderer::Shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		g_pd3dSrvDescHeap->Release();
		g_pd3dSrvDescHeap = nullptr;

		g_pd3dSrvDescHeapAlloc.Destroy();

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
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiRenderer::Draw( float DeltaTime )
	{
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
		ImGui::Render();

		CL->SetDescriptorHeaps( 1, &g_pd3dSrvDescHeap );

		ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), CL );
}

	void ImGuiRenderer::PostExecuteCommands()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}

#endif