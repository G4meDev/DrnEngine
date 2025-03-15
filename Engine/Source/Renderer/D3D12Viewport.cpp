#include "DrnPCH.h"
#include "D3D12Viewport.h"
#include "D3D12Adapter.h"

namespace Drn
{
	D3D12Viewport::D3D12Viewport(D3D12Adapter* InAdapter, HWND InWindowHandle, UINT InSizeX, UINT InSizeY, bool InFullScreen, EPixelFormat InPixelFormat)
		: Adapter(InAdapter)
		, WindowHandle(InWindowHandle)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
		, bFullScreen(InFullScreen)
		, PixelFormat(InPixelFormat)
	{
		Adapter->GetViewports().push_back(this);
	}

	void D3D12Viewport::Init()
	{
		
	}
}