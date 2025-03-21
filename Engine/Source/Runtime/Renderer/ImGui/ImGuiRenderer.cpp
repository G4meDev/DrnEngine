#include "DrnPCH.h"
#include "ImGuiRenderer.h"

//#if WITH_EDITOR

#include "ImGuiLayer.h"
#include "Runtime/Renderer/Renderer.h"


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

	void ImGuiRenderer::Init( dx12lib::Device* InDevice, ID3D12Resource* InViewportResource )
	{
		bInitalized = true;

		m_Device = InDevice;

		ViewportResource = InViewportResource;

		Width = 1920;
		Height = 1080;

		D3D12_COMMAND_QUEUE_DESC desc4 = {};
		desc4.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc4.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc4.NodeMask                 = 1;

		//m_Device->GetD3D12Device()->CreateCommandQueue( &desc4, IID_PPV_ARGS( &g_pd3dCommandQueue ) );
		//m_Device->GetD3D12Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
		//m_Device->GetD3D12Device()->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, nullptr, IID_PPV_ARGS( &g_pd3dCommandList ));
		//g_pd3dCommandList->Close();


		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors             = 64;
		desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_Device->GetD3D12Device()->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &g_pd3dSrvDescHeap ) ); 
		g_pd3dSrvDescHeapAlloc.Create( m_Device->GetD3D12Device().Get(), g_pd3dSrvDescHeap );


		g_pd3dSrvDescHeapAlloc.Alloc(&ViewCpuHandle, &ViewGpuHandle);

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		m_Device->GetD3D12Device()->CreateShaderResourceView(ViewportResource, &descSRV, ViewCpuHandle);

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

		auto Wind = GameFramework::Get().GetWindowByName( L"Clear Screen" );
		ImGui_ImplWin32_Init(Wind->GetWindowHandle());

		ImGui_ImplDX12_InitInfo init_info = {};
		//init_info.UserData = (void*)(SrvHeap.get());



		init_info.Device = m_Device->GetD3D12Device().Get();
        //init_info.CommandQueue         = g_pd3dCommandQueue;
        init_info.CommandQueue         = m_Device->GetCommandQueue().GetD3D12CommandQueue().Get();
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




		{
			m_Device->GetD3D12Device()->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );
			m_fenceValue = 1;

			m_fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
			if ( m_fenceEvent == nullptr )
			{
				HRESULT_FROM_WIN32( GetLastError() );
			}

			//WaitForPreviousFrame();
		}
	}

	void ImGuiRenderer::Tick( float DeltaTime, D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL)
	{
		BeginDraw();
		Draw();
        EndDraw( SwapChainCpuhandle, CL);
	}

	void ImGuiRenderer::AttachLayer(ImGuiLayer* InLayer)
	{
		Layers.AddFirst(InLayer);
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

	void ImGuiRenderer::Draw( )
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		//ImGui::DockSpaceOverViewport();

// 		static bool show_demo_window = true;
// 
// 		if (show_demo_window)
// 			ImGui::ShowDemoWindow(&show_demo_window);
// 		{
// 			static float f = 0.0f;
// 			static int counter = 0;
// 
// 			ImGui::Begin("Hello, world!");
// 
// 			ImGui::Text("This is some useful text.");
// 			ImGui::Checkbox("Demo Window", &show_demo_window);
// 
// 			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
// 
// 			if (ImGui::Button("Button"))
// 				counter++;
// 			ImGui::SameLine();
// 			ImGui::Text("counter = %d", counter);
// 
// 			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
// 			ImGui::End();
// 		}

		ImGui::Begin( "WWW" );
		//ImGui::Image((ImTextureID)ViewGpuHandle.ptr, ImVec2(Width, Height));
		ImGui::End();

// 		for (LinkedListIterator It(Layers); It; ++It)
// 		{
// 			if (It)
// 			{
// 				It->Draw();
// 			}
// 		}
	}

	void ImGuiRenderer::EndDraw( D3D12_CPU_DESCRIPTOR_HANDLE SwapChainCpuhandle, ID3D12GraphicsCommandList* CL )
	{
		//auto& commandQueue = m_Device->GetCommandQueue();
		//auto  commandList  = commandQueue.GetCommandList();

		//CommandAllocator->Reset();
		//g_pd3dCommandList->Reset(CommandAllocator, nullptr);
		//auto commandList = m_Device->GetCommandQueue().GetCommandList()->GetD3D12CommandList();

		//if (ViewportSizeDirty)
		//{
		//D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		//
		//descSRV.Texture2D.MipLevels       = 1;
		//descSRV.Texture2D.MostDetailedMip = 0;
		//descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		//descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		//descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//
		//m_Device->GetD3D12Device()->CreateShaderResourceView( ViewportResource, &descSRV,
		//														ViewCpuHandle );
		//}

		ImGui::Render();


		//g_pd3dCommandList->OMSetRenderTargets(1, &SwapChainCpuhandle, false, nullptr);
		//g_pd3dCommandList->SetDescriptorHeaps( 1, &g_pd3dSrvDescHeap );
		CL->SetDescriptorHeaps( 1, &g_pd3dSrvDescHeap );

		ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), CL );
		

		//ID3D12CommandList* commands[] = { g_pd3dCommandList };
		//g_pd3dCommandList->Close();
		//g_pd3dCommandQueue->ExecuteCommandLists(1, commands);

		//WaitForPreviousFrame();

		//PostExecuteCommands();
	}

	void ImGuiRenderer::WaitForPreviousFrame() 
	{
		const UINT64 fence = m_fenceValue;
		g_pd3dCommandQueue->Signal( m_fence.Get(), fence );
		m_fenceValue++;

		if ( m_fence->GetCompletedValue() < fence )
		{
			m_fence->SetEventOnCompletion( fence, m_fenceEvent );
			WaitForSingleObject( m_fenceEvent, INFINITE );
		}
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

	void ImGuiRenderer::OnViewportResize( float InWidth, float InHeight, ID3D12Resource* InView ) 
	{
		if (!bInitalized)
		{
			return;
		}

		ViewportResource = InView;

		Width = InWidth;
		Height = InHeight;

		ViewportSizeDirty = true;
	}

        }  // namespace Drn

//#endif