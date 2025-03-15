#pragma once

#define NUM_BACKBUFFERS 3

namespace Drn
{
	class D3D12Adapter;
	class D3D12Texture;
	class D3D12Queue;
	class D3D12DescriptorHeap;
	class D3D12CommandAllocator;

	class D3D12Viewport
	{
	public:

		D3D12Viewport(D3D12Adapter* InAdapter, HWND InWindowHandle, UINT InSizeX, UINT InSizeY, bool InFullScreen, DXGI_FORMAT InPixelFormat);

		void Init();

		void Tick(float DeltaTime);

		inline D3D12Queue* GetQueue_Direct() { return CommandQueue_Direct; }
		inline D3D12Queue* GetQueue_Copy() { return CommandQueue_Copy; }
		inline D3D12Queue* GetQueue_Compute() { return CommandQueue_Compute; }


	protected:

		Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;

		D3D12Adapter* Adapter;
		HWND WindowHandle;
		UINT SizeX;
		UINT SizeY;
		DXGI_FORMAT PixelFormat;
		bool bFullScreen;

		UINT BackBufferIndex;

		CD3DX12_VIEWPORT Viewport;
		CD3DX12_RECT ScissorRect;

	private:
		
		Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;

		Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
		ULONG FenceValue;
		HANDLE FenceEvent;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

		std::shared_ptr<D3D12CommandAllocator> CommandAllocator_Direct;

		//std::shared_ptr<D3D12Texture> BackBuffers[NUM_BACKBUFFERS];
		std::shared_ptr<ID3D12Resource> BackBuffers[NUM_BACKBUFFERS];

		D3D12DescriptorHeap* SwapChainDescriptorRVTRoot;
		std::shared_ptr<D3D12DescriptorHeap> SwapChainDescriptorRVT[NUM_BACKBUFFERS];

		D3D12Queue* CommandQueue_Direct;
		D3D12Queue* CommandQueue_Copy;
		D3D12Queue* CommandQueue_Compute;

		std::shared_ptr<D3D12CommandAllocator> CommandAllocators_Direct[NUM_BACKBUFFERS];
		std::shared_ptr<D3D12CommandAllocator> CommandAllocators_Copy[NUM_BACKBUFFERS];
		std::shared_ptr<D3D12CommandAllocator> CommandAllocators_Compute[NUM_BACKBUFFERS];


		void WaitForPreviousFrame();
	};
}