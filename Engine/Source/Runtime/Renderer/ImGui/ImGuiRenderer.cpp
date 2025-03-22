#include "DrnPCH.h"
#include "ImGuiRenderer.h"

#if WITH_EDITOR

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

	void ImGuiRenderer::Init(ID3D12Resource* InViewportResource )
	{
		bInitalized = true;

		ID3D12Device* pDevice = Renderer::Get()->GetDevice()->GetD3D12Device().Get();
		ViewportResource = InViewportResource;

		Width = 1920;
		Height = 1080;

		D3D12_COMMAND_QUEUE_DESC desc4 = {};
		desc4.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc4.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc4.NodeMask                 = 1;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors             = 64;
		desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		pDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &g_pd3dSrvDescHeap ) ); 
		g_pd3dSrvDescHeapAlloc.Create( pDevice, g_pd3dSrvDescHeap );


		g_pd3dSrvDescHeapAlloc.Alloc(&ViewCpuHandle, &ViewGpuHandle);

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		pDevice->CreateShaderResourceView(ViewportResource, &descSRV, ViewCpuHandle);

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
		//ImGui::DockSpaceOverViewport();

		ImGui::Begin( "WWW" );

		const ImVec2   AvaliableSize = ImGui::GetContentRegionAvail();
		IntPoint ImageSize     = IntPoint( (int)AvaliableSize.x, (int)AvaliableSize.y );

		ImageSize.X = std::max(ImageSize.X, 1);
		ImageSize.Y = std::max(ImageSize.Y, 1);

		if ( CachedSize != ImageSize )
		{
			CachedSize = ImageSize;
			Renderer::Get()->ViewportResized(CachedSize.X, CachedSize.Y);
		}

		ImGui::Image((ImTextureID)ViewGpuHandle.ptr, ImVec2(Width, Height));
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
		if (ViewportSizeDirty)
		{
		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		
		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		Renderer::Get()->GetDevice()->GetD3D12Device()->CreateShaderResourceView( ViewportResource, &descSRV,
																ViewCpuHandle );
		}

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

	void ImGuiRenderer::OnViewportResize( float InWidth, float InHeight, ID3D12Resource* InView ) 
	{
		if (!bInitalized)
		{
			return;
		}

		ViewportResource = InView;

		Width = std::max(InWidth, 1.0f);
		Height = std::max( InHeight, 1.0f );

		ViewportSizeDirty = true;
	}
}

#endif