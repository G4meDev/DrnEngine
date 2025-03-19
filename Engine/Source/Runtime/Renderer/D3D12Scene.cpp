#include "DrnPCH.h"
#include "D3D12Scene.h"

#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
 
#include "TestShader_PS.h"
#include "TestShader_VS.h"

namespace Drn
{
	D3D12Scene::D3D12Scene(HWND InWindowHandle, const IntPoint& InSize, bool InFullScreen, DXGI_FORMAT InPixelFormat)
		: WindowHandle(InWindowHandle)
		, Size(InSize)
		, bFullScreen(InFullScreen)
		, PixelFormat(InPixelFormat)
	{
		Init();
	}

	void D3D12Scene::Init()
	{
// 		Resize(Size);
// 
// 		// -------------------------------------------------------------------------------------------------
// 
// 		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
// 		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
// 
// 		Microsoft::WRL::ComPtr<ID3DBlob> signature;
// 		Microsoft::WRL::ComPtr<ID3DBlob> error;
// 		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
// 		Adapter->GetD3DDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetAddressOf()));
// 
// 		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
// 		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
// 
// #if DRN_DEBUG
// 		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
// #else
// 		UINT compileFlags = 0;
// #endif
// 
// 		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
// 		{
// 			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
// 			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
// 		};
// 
// 		D3D12_GRAPHICS_PIPELINE_STATE_DESC BasePasspsoDesc = {};
// 		BasePasspsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
// 		BasePasspsoDesc.pRootSignature = RootSignature.Get();
// 		BasePasspsoDesc.VS = CD3DX12_SHADER_BYTECODE(TestShader_VS, sizeof(TestShader_VS));
// 		BasePasspsoDesc.PS = CD3DX12_SHADER_BYTECODE(TestShader_PS, sizeof(TestShader_PS));
// 		BasePasspsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
// 		BasePasspsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
// 		BasePasspsoDesc.DepthStencilState.DepthEnable = FALSE;
// 		BasePasspsoDesc.DepthStencilState.StencilEnable = FALSE;
// 		BasePasspsoDesc.SampleMask = UINT_MAX;
// 		BasePasspsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
// 		BasePasspsoDesc.NumRenderTargets = 1;
// 		BasePasspsoDesc.RTVFormats[0] = PixelFormat;
// 		BasePasspsoDesc.SampleDesc.Count = 1;
//  		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateGraphicsPipelineState(&BasePasspsoDesc, IID_PPV_ARGS(BasePassPipelineState.GetAddressOf())));
// 
// 		// -------------------------------------------------------------------------------------------------
// 
// 		struct Vertex
// 		{
// 			float position[4];
// 			float color[4];
// 		};
// 
// 		Vertex triangleVertices[] =
// 		{
// 			{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
// 			{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
// 			{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
// 		};
// 
// 		const UINT vertexBufferSize = sizeof(triangleVertices);
// 
// 		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateCommittedResource(
// 			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
// 			D3D12_HEAP_FLAG_NONE,
// 			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
// 			D3D12_RESOURCE_STATE_GENERIC_READ,
// 			nullptr,
// 			IID_PPV_ARGS(VertexBuffer.GetAddressOf())));
// 
// 		UINT8* pVertexDataBegin;
// 		CD3DX12_RANGE readRange(0, 0);
// 		VERIFYD3D12RESULT(VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
// 		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
// 		VertexBuffer->Unmap(0, nullptr);
// 
// 		VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
// 		VertexBufferView.StrideInBytes = sizeof(Vertex);
// 		VertexBufferView.SizeInBytes = vertexBufferSize;

		// -------------------------------------------------------------------------------------------------

	}

	void D3D12Scene::Tick(float DeltaTime)
	{
// 		auto* CommandList = Renderer::Get()->GetCommandList();
//  		CommandList->SetPipelineState(BasePassPipelineState.Get());
// 
// 		CommandList->SetGraphicsRootSignature(RootSignature.Get());
// 		CommandList->RSSetViewports(1, &Viewport);
// 		CommandList->RSSetScissorRects(1, &ScissorRect);
// 
//  		ID3D12Resource* BasePassResource = BasePassBuffer.Get();
// 		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BasePassResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
// 		CD3DX12_CPU_DESCRIPTOR_HANDLE BasePassrtvHandle = BasePassRTV->GetCpuHandle();
// 
// 		CommandList->OMSetRenderTargets(1, &BasePassrtvHandle, FALSE, nullptr);
// 
// 		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
// 		CommandList->ClearRenderTargetView(BasePassrtvHandle, clearColor, 0, nullptr);
// 		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
// 		CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
// 		CommandList->DrawInstanced(3, 1, 0, 0);
// 
// 		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BasePassResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
 	}

	void D3D12Scene::Resize(const IntPoint& InSize)
	{
// 		Size = IntPoint(std::max(InSize.X, 1), std::max(InSize.Y, 1));
// 
// 		Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Size.X), static_cast<float>(Size.Y));
// 		ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(Size.X), static_cast<LONG>(Size.Y));
// 		
// 		D3D12_CLEAR_VALUE BasePassClearValue = {};
// 		BasePassClearValue.Format = PixelFormat;
// 		BasePassClearValue.Color[0] = 0.0f;
// 		BasePassClearValue.Color[1] = 0.2f;
// 		BasePassClearValue.Color[2] = 0.4f;
// 		BasePassClearValue.Color[3] = 1.0f;
// 
// 
// 		if (BasePassRTV.get() == nullptr)
// 		{
// 			BasePassRTV = std::make_unique<D3D12DescriptorHeap>(Adapter->GetDevice(), 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, ED3D12DescriptorHeapFlags::None, false);
// 		}
// 		
// 		VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateCommittedResource(
// 			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
// 			D3D12_HEAP_FLAG_NONE,
// 			&CD3DX12_RESOURCE_DESC::Tex2D(PixelFormat, Size.X, Size.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
// 			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
// 			&BasePassClearValue,
// 			IID_PPV_ARGS(BasePassBuffer.GetAddressOf())));
// 
// 		D3D12_RENDER_TARGET_VIEW_DESC BasePassrtvDesc = {};
// 		BasePassrtvDesc.Format = PixelFormat;
// 		BasePassrtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
// 
// 		Adapter->GetD3DDevice()->CreateRenderTargetView(BasePassBuffer.Get(), &BasePassrtvDesc, BasePassRTV->GetCpuHandle());
// 
// 		BasePassBuffer->SetName(L"BasePassBuffer");
	}
}