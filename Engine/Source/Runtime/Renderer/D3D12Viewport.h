#pragma once

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
		inline ID3D12GraphicsCommandList* GetCommandList() { return CommandList.Get(); }

		inline ID3D12Resource* GetOutputBuffer() { return BasePassBuffer.Get(); }


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
		Microsoft::WRL::ComPtr<ID3D12PipelineState> BasePassPipelineState;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> ResolvePipelineState;
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

		std::shared_ptr<D3D12DescriptorHeap> BasePassRTV;
		Microsoft::WRL::ComPtr<ID3D12Resource> BasePassBuffer;

		D3D12Queue* CommandQueue_Direct;

		void WaitForPreviousFrame();
	};
}