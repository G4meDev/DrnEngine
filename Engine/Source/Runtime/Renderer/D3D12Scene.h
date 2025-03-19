#pragma once

namespace Drn
{
	class D3D12Scene
	{
	public:

		D3D12Scene(HWND InWindowHandle, const IntPoint& InSize, bool InFullScreen, DXGI_FORMAT InPixelFormat);

		void Init();

		void Tick(float DeltaTime);

		inline IntPoint GetSize() { return Size; };

		void Resize(const IntPoint& InSize);

	protected:

		HWND WindowHandle;
		IntPoint Size;
		DXGI_FORMAT PixelFormat;
		bool bFullScreen;

		CD3DX12_VIEWPORT Viewport;
		CD3DX12_RECT ScissorRect;

	private:

	};
}