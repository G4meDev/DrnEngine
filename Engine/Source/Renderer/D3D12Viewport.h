#pragma once

#define NUM_BACKBUFFERS 3

namespace Drn
{
	class D3D12Adapter;
	class D3D12Texture;
	class D3D12Queue;
	class D3D12DescriptorHeap;

	class D3D12Viewport
	{
	public:

		D3D12Viewport(D3D12Adapter* InAdapter, HWND InWindowHandle, UINT InSizeX, UINT InSizeY, bool InFullScreen, DXGI_FORMAT InPixelFormat);

		void Init();

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

	private:

		//std::shared_ptr<D3D12Texture> BackBuffers[NUM_BACKBUFFERS];
		std::shared_ptr<ID3D12Resource> BackBuffers[NUM_BACKBUFFERS];

		D3D12DescriptorHeap* SwapChainDescriptorRVTRoot;
		std::shared_ptr<D3D12DescriptorHeap> SwapChainDescriptorRVT[NUM_BACKBUFFERS];

		D3D12Queue* CommandQueue_Direct;
		D3D12Queue* CommandQueue_Copy;
		D3D12Queue* CommandQueue_Compute;

	};
}