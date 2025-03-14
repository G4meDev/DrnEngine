#include "DrnPCH.h"
#include "Window.h"

namespace Drn
{
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LONG_PTR WindowPtr = GetWindowLongPtr(hWnd, /*GWL_USERDATA*/ (-21));
		Window* Win = reinterpret_cast<Drn::Window*>(WindowPtr);

		switch (message)
		{
		case WM_SIZE:
			
			Win->Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}

	void Window::Init()
	{
		std::cout << "Start window" << std::endl;

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInstance;
		wcex.hIcon = LoadIcon(m_hInstance, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = Title.c_str();
		wcex.hIconSm = LoadIcon(m_hInstance, IDI_APPLICATION);

		// TODO: error
		//ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");
		bool result = RegisterClassEx(&wcex);


		RECT rc = { 0, 0, (LONG)Width, (LONG)Height };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		m_WindowHandle = CreateWindow(Title.c_str(), Title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, m_hInstance, nullptr);
		// TODO: error

		SetWindowLongPtr(m_WindowHandle, /*GWL_USERDATA*/ (-21), LONG_PTR(this));

		ShowWindow(m_WindowHandle, SW_SHOWDEFAULT);
	}

	void Window::Shutdown()
	{
		std::cout << "Shutdown window" << std::endl;
	}

	void Window::Tick(float DeltaTime)
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				g_bPendingClose = true;
		}
	}

	void Window::Resize(int16 InWidth, int16 InHeight)
	{
		std::cout << "Resize to '" << InWidth << "x" << InHeight << std::endl;

		Width = InWidth;
		Height = InHeight;
	}

	bool Window::PendingClose() const
	{
		return g_bPendingClose;
	}

}