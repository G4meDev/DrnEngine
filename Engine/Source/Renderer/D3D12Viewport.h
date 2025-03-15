#pragma once

#include "PixelFormat.h"

namespace Drn
{
	class D3D12Adapter;
	class D3D12Texture;

	class D3D12Viewport
	{
	public:

		D3D12Viewport(D3D12Adapter* InAdapter, HWND InWindowHandle, UINT InSizeX, UINT InSizeY, bool InFullScreen, EPixelFormat InPixelFormat);

		void Init();


	protected:

		D3D12Adapter* Adapter;
		HWND WindowHandle;
		UINT SizeX;
		UINT SizeY;
		EPixelFormat PixelFormat;
		bool bFullScreen;

	private:

		struct BackBufferData
		{
		public:
			std::shared_ptr<D3D12Texture> Texture;
		};
	};
}