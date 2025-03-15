#pragma once

#include "DrnPCH.h"

namespace Drn
{
	class D3D12Adapter;
	class D3D12Viewport;

	class Window
	{
	public:
		Window(HINSTANCE InhInstance, D3D12Adapter* InAdapter, std::wstring& InTitle, int16 InWidth = 800, int16 InHeight = 600);

		~Window()
		{
			Shutdown();
		}

	public:
		void Shutdown();

		void Tick(float DeltaTime);

		void Resize(int16 InWidth, int16 InHeight);

		bool PendingClose() const; 
	
	protected:
		uint16 Width;
		uint16 Height;

		std::wstring Title;

		HINSTANCE m_hInstance;
		HWND m_WindowHandle;

		bool g_bPendingClose = false;

		D3D12Viewport* Viewport;

	private:
	};
}

