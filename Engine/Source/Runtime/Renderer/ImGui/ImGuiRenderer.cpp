#include "DrnPCH.h"
#include "ImGuiRenderer.h"

#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/D3D12Adapter.h"
#include "Runtime/Renderer/D3D12Descriptors.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

namespace Drn
{
	std::unique_ptr<ImGuiRenderer> ImGuiRenderer::SingletonInstance = nullptr;

	ImGuiRenderer::ImGuiRenderer()
	{

	}

	ImGuiRenderer::~ImGuiRenderer()
	{

	}

	void ImGuiRenderer::Init(ID3D12Resource* MainViewportOutputBuffer)
	{
		SrvHeap = std::make_unique<D3D12DescriptorHeap>(Renderer::Get()->Adapter->GetDevice(), 64, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ED3D12DescriptorHeapFlags::GpuVisible, false);
		MainViewportOutputHeap = std::make_unique<D3D12DescriptorHeap>(SrvHeap.get());

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Renderer::Get()->Adapter->GetD3DDevice()->CreateShaderResourceView(MainViewportOutputBuffer, &descSRV, MainViewportOutputHeap->GetCpuHandle());

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

		ImGui_ImplWin32_Init(Renderer::Get()->GetMainWindow()->GetWindowHandle());

		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.UserData = (void*)(SrvHeap.get());
		init_info.Device = Renderer::Get()->Adapter->GetD3DDevice();
		init_info.CommandQueue = Renderer::Get()->GetCommandQueue();
		init_info.NumFramesInFlight = NUM_BACKBUFFERS;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		init_info.SrvDescriptorHeap = SrvHeap->GetHeap();
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* Info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
			{
				D3D12DescriptorHeap* H = new D3D12DescriptorHeap(static_cast<D3D12DescriptorHeap*>(Info->UserData));

				out_cpu_handle->ptr = H->GetCpuHandle().ptr;
				out_gpu_handle->ptr = H->GetGpuHandle().ptr;
			};
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* Info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
			{
				return static_cast<D3D12DescriptorHeap*>(Info->UserData)->Free(cpu_handle);
			};

		ImGui_ImplDX12_Init(&init_info);
	}

	void ImGuiRenderer::Tick(float DeltaTime)
	{
		BeginDraw();
		Draw();
		EndDraw();
	}

	void ImGuiRenderer::AttachLayer(ImGuiLayer* InLayer)
	{

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

	void ImGuiRenderer::Draw()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		static bool show_demo_window = true;
		static bool show_another_window = false;

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");

			ImGui::Text("This is some useful text.");
			ImGui::Checkbox("Demo Window", &show_demo_window);
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

			if (ImGui::Button("Button"))
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();

			ImGui::Begin("MainWindow");
			ImGui::Image(ImTextureID(MainViewportOutputHeap->GetGpuHandle().ptr), ImVec2(Renderer::Get()->GetMainWindow()->GetSizeX(), Renderer::Get()->GetMainWindow()->GetSizeY()));
			ImGui::End();
		}

		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

// 		for (LinkedListIterator It(Layers); It; ++It)
// 		{
// 			if (It)
// 			{
// 				It->Draw();
// 			}
// 		}
	}

	void ImGuiRenderer::EndDraw()
	{
		ImGui::Render();

		ID3D12DescriptorHeap* SRV = SrvHeap->GetHeap();
		Renderer::Get()->GetCommandList()->SetDescriptorHeaps(1, &SRV);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Renderer::Get()->GetCommandList());
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