#pragma once

#include "DrnPCH.h"

namespace Drn
{
	class Window
	{
	public:
		Window(HINSTANCE InhInstance, std::wstring& InTitle, int16 InWidth = 800, int16 InHeight = 600)
		: m_hInstance(InhInstance)
		, Title(InTitle)
		, Width(InWidth)
		, Height(InHeight)
		{
			Init();
		}

		~Window()
		{
			Shutdown();
		}

	public:

		void Init();
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

	private:
	};
}

