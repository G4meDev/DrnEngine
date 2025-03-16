#include "DrnPCH.h"
#include "D3D12Device.h"
#include "D3D12Viewport.h"
#include "D3D12Adapter.h"
#include "D3D12Queue.h"
#include "D3D12Descriptors.h"

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
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

		D3D12_CLEAR_VALUE BasePassClearValue = {};
		BasePassClearValue.Format = PixelFormat;
		BasePassClearValue.Color[0] = 0.0f;
		BasePassClearValue.Color[1] = 0.2f;
		BasePassClearValue.Color[2] = 0.4f;
		BasePassClearValue.Color[3] = 1.0f;

		BasePassRTV = std::make_shared<D3D12DescriptorHeap>(Adapter->GetDevice(), 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, ED3D12DescriptorHeapFlags::None, false);
		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(PixelFormat, SizeX, SizeY, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&BasePassClearValue,
			IID_PPV_ARGS(BasePassBuffer.GetAddressOf())));

 		D3D12_RENDER_TARGET_VIEW_DESC BasePassrtvDesc = {};
 		BasePassrtvDesc.Format = PixelFormat;
 		BasePassrtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
 
 		Adapter->GetD3DDevice()->CreateRenderTargetView(BasePassBuffer.Get(), &BasePassrtvDesc, BasePassRTV->GetCpuHandle());
 
 		BasePassBuffer->SetName(L"BasePassBuffer");

		SwapChainDescriptorRVTRoot = new D3D12DescriptorHeap(Adapter->GetDevice(), NUM_BACKBUFFERS, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, ED3D12DescriptorHeapFlags::None, false);

		for (UINT n = 0; n < NUM_BACKBUFFERS; n++)
		{
			ID3D12Resource* RenderTargetPtr;
			SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargetPtr));

			wchar_t name[25] = {};
			swprintf_s(name, L"Render target %u", n);
			RenderTargetPtr->SetName(name);

			BackBuffers[n] = std::shared_ptr<ID3D12Resource>(RenderTargetPtr);

			D3D12DescriptorHeap* RVT = new D3D12DescriptorHeap(SwapChainDescriptorRVTRoot);
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

 		ID3D12Resource* BasePassResource = BasePassBuffer.Get();
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BasePassResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
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

		ImGuiRenderer::Get()->Tick(DeltaTime);

		// -------------------------------------------------------------------------------------------------

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BackBufferResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		VERIFYD3D12RESULT(CommandList->Close());

		ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
		CommandQueue_Direct->CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		ImGuiRenderer::Get()->PostExecuteCommands();

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