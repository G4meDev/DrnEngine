#pragma once

namespace Drn
{
	class D3D12Adapter;
	class D3D12Texture;
	class D3D12Queue;
	class D3D12DescriptorHeap;
	class D3D12CommandAllocator;

	class D3D12Scene
	{
	public:

		D3D12Scene(D3D12Adapter* InAdapter, HWND InWindowHandle, const IntPoint& InSize, bool InFullScreen, DXGI_FORMAT InPixelFormat);

		void Init();

		void Tick(float DeltaTime);

		inline ID3D12Resource* GetOutputBuffer() { return BasePassBuffer.Get(); }
		inline IntPoint GetSize() { return Size; };

		void Resize(const IntPoint& InSize);

	protected:

		D3D12Adapter* Adapter;
		HWND WindowHandle;
		IntPoint Size;
		DXGI_FORMAT PixelFormat;
		bool bFullScreen;

		CD3DX12_VIEWPORT Viewport;
		CD3DX12_RECT ScissorRect;

	private:

		Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> BasePassPipelineState;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> ResolvePipelineState;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

		std::unique_ptr<D3D12DescriptorHeap> BasePassRTV;
		Microsoft::WRL::ComPtr<ID3D12Resource> BasePassBuffer;
	};
}