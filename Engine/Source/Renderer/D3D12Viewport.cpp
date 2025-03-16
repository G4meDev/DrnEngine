#include "DrnPCH.h"
#include "D3D12Device.h"
#include "D3D12Viewport.h"
#include "D3D12Adapter.h"
#include "D3D12Queue.h"
#include "D3D12Descriptors.h"

//#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

namespace Drn
{
	ExampleDescriptorHeapAllocator D3D12Viewport::g_pd3dSrvDescHeapAlloc;

	D3D12Viewport::D3D12Viewport(D3D12Adapter* InAdapter, HWND InWindowHandle, UINT InSizeX, UINT InSizeY, bool InFullScreen, DXGI_FORMAT InPixelFormat)
		: Adapter(InAdapter)
		, WindowHandle(InWindowHandle)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
		, bFullScreen(InFullScreen)
		, PixelFormat(InPixelFormat)
		, Viewport(0.0f, 0.0f, static_cast<float>(InSizeX), static_cast<float>(InSizeY))
		, ScissorRect(0, 0, static_cast<LONG>(InSizeX), static_cast<LONG>(InSizeY))
	{
		Adapter->GetViewports().push_back(this);
		Init();
	}

	void D3D12Viewport::Init()
	{
		CommandQueue_Direct = new D3D12Queue(Adapter->GetDevice(), D3D12QueueType::Direct);
		CommandQueue_Copy = new D3D12Queue(Adapter->GetDevice(), D3D12QueueType::Copy);
		CommandQueue_Compute = new D3D12Queue(Adapter->GetDevice(), D3D12QueueType::Compute);

		for (UINT n = 0; n < NUM_BACKBUFFERS; n++)
		{
			CommandAllocators_Direct[n] = std::make_shared<D3D12CommandAllocator>(D3D12CommandAllocator(Adapter->GetDevice(), D3D12QueueType::Direct));
			CommandAllocators_Copy[n] = std::make_shared<D3D12CommandAllocator>(D3D12CommandAllocator(Adapter->GetDevice(), D3D12QueueType::Copy));
			CommandAllocators_Compute[n] = std::make_shared<D3D12CommandAllocator>(D3D12CommandAllocator(Adapter->GetDevice(), D3D12QueueType::Compute));
		}

		CommandAllocator_Direct = std::make_shared<D3D12CommandAllocator>(D3D12CommandAllocator(Adapter->GetDevice(), D3D12QueueType::Direct));


		UINT SwapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1{};

		SwapChainDesc1.Width = SizeX;
		SwapChainDesc1.Height = SizeY;
		SwapChainDesc1.Format = PixelFormat;
		SwapChainDesc1.SampleDesc.Count = 1;
		SwapChainDesc1.SampleDesc.Quality = 0;
		SwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc1.BufferCount = NUM_BACKBUFFERS;
		SwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		SwapChainDesc1.Flags = SwapChainFlags;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullscreenDesc{};
		FullscreenDesc.RefreshRate.Numerator = 0;
		FullscreenDesc.RefreshRate.Denominator = 0;
		FullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		FullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		FullscreenDesc.Windowed = !bFullScreen;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain1;
		VERIFYD3D12RESULT(Adapter->GetFactory()->CreateSwapChainForHwnd(CommandQueue_Direct->CommandQueue.Get(), WindowHandle, &SwapChainDesc1, &FullscreenDesc, nullptr, SwapChain1.GetAddressOf()));

		SwapChain1.As(&SwapChain);

		BasePassRTV = std::shared_ptr<D3D12DescriptorHeap>(D3D12DescriptorHeap::Create(Adapter->GetDevice(), L"Base pass descriptor", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, ED3D12DescriptorHeapFlags::None, false));
		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(PixelFormat, SizeX, SizeY, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(BasePassBuffer.GetAddressOf())));

		D3D12_RENDER_TARGET_VIEW_DESC BasePassrtvDesc = {};
		BasePassrtvDesc.Format = PixelFormat;
		BasePassrtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		Adapter->GetD3DDevice()->CreateRenderTargetView(BasePassBuffer.Get(), &BasePassrtvDesc, BasePassRTV->GetCpuHandle());

		BasePassBuffer->SetName(L"BasePassBuffer");

		SwapChainDescriptorRVTRoot = D3D12DescriptorHeap::Create(Adapter->GetDevice(), L"Swap chain descriptor", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_BACKBUFFERS, ED3D12DescriptorHeapFlags::None, false);


		D3D12DescriptorHeap* SRVHeapPtr = D3D12DescriptorHeap::Create(Adapter->GetDevice(), L"SRV descriptor", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 64, ED3D12DescriptorHeapFlags::GpuVisible, false);
		DescriptorHeapSRV = std::shared_ptr<D3D12DescriptorHeap>(SRVHeapPtr);

		g_pd3dSrvDescHeapAlloc.Create(Adapter->GetD3DDevice(), DescriptorHeapSRV->GetHeap());
		g_pd3dSrvDescHeapAlloc.Alloc(&SRVCpuBase, &SRVGpuBase);

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format = PixelFormat;
		descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Adapter->GetD3DDevice()->CreateShaderResourceView(BasePassBuffer.Get(), &descSRV, SRVCpuBase);

		for (UINT n = 0; n < NUM_BACKBUFFERS; n++)
		{
			ID3D12Resource* RenderTargetPtr;
			SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargetPtr));

			wchar_t name[25] = {};
			swprintf_s(name, L"Render target %u", n);
			RenderTargetPtr->SetName(name);

			BackBuffers[n] = std::shared_ptr<ID3D12Resource>(RenderTargetPtr);

			D3D12DescriptorHeap* RVT = new D3D12DescriptorHeap(SwapChainDescriptorRVTRoot, n, 1);
			SwapChainDescriptorRVT[n] = std::shared_ptr<D3D12DescriptorHeap>(RVT);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = PixelFormat;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			Adapter->GetD3DDevice()->CreateRenderTargetView(RenderTargetPtr, &rtvDesc, RVT->GetCpuHandle());
		}

		BackBufferIndex = SwapChain->GetCurrentBackBufferIndex();

		// -------------------------------------------------------------------------------------------------

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		Adapter->GetD3DDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf()));

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

#if DRN_DEBUG
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		
		VERIFYD3D12RESULT(D3DCompileFromFile(Path::ShaderFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, vertexShader.GetAddressOf(), nullptr));
		VERIFYD3D12RESULT(D3DCompileFromFile(Path::ShaderFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, pixelShader.GetAddressOf(), nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC BasePasspsoDesc = {};
		BasePasspsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		BasePasspsoDesc.pRootSignature = RootSignature.Get();
		BasePasspsoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		BasePasspsoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		BasePasspsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		BasePasspsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		BasePasspsoDesc.DepthStencilState.DepthEnable = FALSE;
		BasePasspsoDesc.DepthStencilState.StencilEnable = FALSE;
		BasePasspsoDesc.SampleMask = UINT_MAX;
		BasePasspsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		BasePasspsoDesc.NumRenderTargets = 1;
		BasePasspsoDesc.RTVFormats[0] = PixelFormat;
		BasePasspsoDesc.SampleDesc.Count = 1;
 		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateGraphicsPipelineState(&BasePasspsoDesc, IID_PPV_ARGS(BasePassPipelineState.GetAddressOf())));

		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator_Direct->CommandAllocator.Get(), BasePassPipelineState.Get(), IID_PPV_ARGS(CommandList.GetAddressOf())));

// 		D3D12_GRAPHICS_PIPELINE_STATE_DESC ResolvepsoDesc = {};
// 		ResolvepsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
// 		ResolvepsoDesc.pRootSignature = RootSignature.Get();
// 		ResolvepsoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
// 		ResolvepsoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
// 		ResolvepsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
// 		ResolvepsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
// 		ResolvepsoDesc.DepthStencilState.DepthEnable = FALSE;
// 		ResolvepsoDesc.DepthStencilState.StencilEnable = FALSE;
// 		ResolvepsoDesc.SampleMask = UINT_MAX;
// 		ResolvepsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
// 		ResolvepsoDesc.NumRenderTargets = 1;
// 		ResolvepsoDesc.RTVFormats[0] = PixelFormat;
// 		ResolvepsoDesc.SampleDesc.Count = 1;
// 		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateGraphicsPipelineState(&BasePasspsoDesc, IID_PPV_ARGS(BasePassPipelineState.GetAddressOf())));

		VERIFYD3D12RESULT(CommandList->Close());

		// -------------------------------------------------------------------------------------------------

		struct Vertex
		{
			float position[4];
			float color[4];
		};

		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(VertexBuffer.GetAddressOf())));

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		VERIFYD3D12RESULT(VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		VertexBuffer->Unmap(0, nullptr);

		VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		VertexBufferView.StrideInBytes = sizeof(Vertex);
		VertexBufferView.SizeInBytes = vertexBufferSize;

		// -------------------------------------------------------------------------------------------------

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

		ImGui_ImplWin32_Init(WindowHandle);

		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = Adapter->GetD3DDevice();
		init_info.CommandQueue = CommandQueue_Direct->CommandQueue.Get();
		init_info.NumFramesInFlight = NUM_BACKBUFFERS;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		init_info.SrvDescriptorHeap = DescriptorHeapSRV->GetHeap();
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
		ImGui_ImplDX12_Init(&init_info);

		// -------------------------------------------------------------------------------------------------

		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf())));
		FenceValue = 1;

		FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (FenceEvent == nullptr)
		{
			VERIFYD3D12RESULT(HRESULT_FROM_WIN32(GetLastError()));
		}

		WaitForPreviousFrame();
	}

	void D3D12Viewport::Tick(float DeltaTime)
	{
		VERIFYD3D12RESULT(CommandAllocator_Direct->CommandAllocator->Reset());
		VERIFYD3D12RESULT(CommandList->Reset(CommandAllocator_Direct->CommandAllocator.Get(), BasePassPipelineState.Get()));

		CommandList->SetGraphicsRootSignature(RootSignature.Get());
		CommandList->RSSetViewports(1, &Viewport);
		CommandList->RSSetScissorRects(1, &ScissorRect);

 		//ID3D12Resource* BackBufferResource = BackBuffers[BackBufferIndex].get();
 		ID3D12Resource* BasePassResource = BasePassBuffer.Get();
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BasePassResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = SwapChainDescriptorRVT[BackBufferIndex]->GetCpuHandle();
		CD3DX12_CPU_DESCRIPTOR_HANDLE BasePassrtvHandle = BasePassRTV->GetCpuHandle();

		CommandList->OMSetRenderTargets(1, &BasePassrtvHandle, FALSE, nullptr);

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		CommandList->ClearRenderTargetView(BasePassrtvHandle, clearColor, 0, nullptr);
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
		CommandList->DrawInstanced(3, 1, 0, 0);

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BasePassResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		ID3D12Resource* BackBufferResource = BackBuffers[BackBufferIndex].get();
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BackBufferResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = SwapChainDescriptorRVT[BackBufferIndex]->GetCpuHandle();
		CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// -------------------------------------------------------------------------------------------------

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		static bool show_demo_window = true;
		static bool show_another_window = false;
		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//ImGui::DockSpaceOverViewport();

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
			ImGui::ColorEdit3("clear color", (float*)&clear_color);

			if (ImGui::Button("Button"))
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();

			ImGui::Begin("MainWindow");
			ImGui::Image(ImTextureID(SRVGpuBase.ptr), ImVec2(SizeX, SizeY));
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

		ImGui::Render();

		ID3D12DescriptorHeap* SRV = DescriptorHeapSRV->GetHeap();
		CommandList->SetDescriptorHeaps(1, &SRV);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CommandList.Get());

		// -------------------------------------------------------------------------------------------------

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BackBufferResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		VERIFYD3D12RESULT(CommandList->Close());

		ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
		CommandQueue_Direct->CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

 		VERIFYD3D12RESULT(SwapChain->Present(1, 0));
 
		WaitForPreviousFrame();
	}

	void D3D12Viewport::WaitForPreviousFrame()
	{
		const UINT64 fence = FenceValue;
		VERIFYD3D12RESULT(CommandQueue_Direct->CommandQueue->Signal(Fence.Get(), fence));
		FenceValue++;

		if (Fence->GetCompletedValue() < fence)
		{
			VERIFYD3D12RESULT(Fence->SetEventOnCompletion(fence, FenceEvent));
			WaitForSingleObject(FenceEvent, INFINITE);
		}

		BackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
	}
}