#include "DrnPCH.h"
#include "Window.h"

LOG_DEFINE_CATEGORY( LogWindow, "Window" );

#if WITH_EDITOR
#include <imgui.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
#endif

namespace Drn
{
	Window::Window( HINSTANCE hInstance, const std::wstring& ClassName, const std::wstring& WindowName, const IntPoint& WindowSize )
		: m_Closing(false)
		, m_Borderless(false)
	{
		RECT WindowRect = { 0, 0, static_cast<LONG>( WindowSize.X ), static_cast<LONG>( WindowSize.Y) };
		AdjustWindowRect( &WindowRect, m_windowStyle, FALSE );

		IntPoint AdjustedSize( WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top );
		// maybe is should use adjusted values?
		m_WindowSize = WindowSize;

		IntPoint ScreenSize( GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ) );
		IntPoint WindowPosition = (ScreenSize - AdjustedSize) / 2;
		WindowPosition = IntPoint::ComponentWiseMax(WindowPosition, IntPoint(0));

		m_Name = WindowName;
		m_hWnd = CreateWindowExW( NULL, ClassName.c_str(), WindowName.c_str(), WS_OVERLAPPEDWINDOW, WindowPosition.X,
											WindowPosition.Y, AdjustedSize.X, AdjustedSize.Y, NULL, NULL, hInstance, NULL );

		if ( !m_hWnd )
		{
			LOG(LogWindow, Error, "faield to create window.");
			return;
		}

		SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

		SetWindowTitle(WindowName);
	}

	Window::~Window()
	{
		DestroyWindow( m_hWnd );
	}

	void Window::RegisterDefaultClass( HINSTANCE hInstance )
	{
		WNDCLASSEXW wndClass = { 0 };

		wndClass.cbSize        = sizeof( WNDCLASSEX );
		wndClass.style         = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc   = &Window::DefaultWndProc;
		wndClass.hInstance     = hInstance;
		wndClass.hCursor       = LoadCursor( nullptr, IDC_ARROW );
		wndClass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
		wndClass.lpszMenuName  = nullptr;
		wndClass.lpszClassName = DEFAULT_WINDOW_CLASS_NAME;
		//wndClass.hIcon         = LoadIcon( hInstance, MAKEINTRESOURCE( APP_ICON ) );
		//wndClass.hIconSm       = LoadIcon( m_hInstance, MAKEINTRESOURCE( APP_ICON ) );
		wndClass.hIcon         = nullptr;
		wndClass.hIconSm       = nullptr;

		if ( !RegisterClassExW( &wndClass ) )
		{
			LOG( LogWindow, Error, "unable to register the window class.");
		}
	}

	void Window::SetWindowTitle( const std::wstring& windowTitle )
	{
		m_Title = windowTitle;
		SetWindowTextW( m_hWnd, m_Title.c_str() );
	}


	void Window::SetWindowSize( const IntPoint& windowTitle )
	{
		
	}

	bool Window::IsMaximaized() const
	{
		WINDOWPLACEMENT Plc;
		GetWindowPlacement(m_hWnd, &Plc);

		return Plc.showCmd == SW_MAXIMIZE;
	}

	void Window::SetMaximaized( bool bMaximazied )
	{
		ShowWindow( m_hWnd, bMaximazied ? SW_MAXIMIZE : SW_NORMAL );
	}

	void Window::ToggleMaximaized()
	{
		SetMaximaized(!IsMaximaized());
	}

	bool Window::IsBorderlessFullScreen() const
	{
		return m_Borderless;
	}

	void Window::SetBorderlessFullScreen( bool bBorderless )
	{
		m_Borderless = bBorderless;

		if (m_Borderless)
		{
			SetWindowLong(m_hWnd, GWL_STYLE, m_windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));
			RECT FullScreenRect;

			const bool RectFromSwapchainSuccessed = Renderer::Get() && Renderer::Get()->GetSwapchainContainingRect(FullScreenRect);

			if (!RectFromSwapchainSuccessed)
			{
				drn_check(false);
				// TODO: add fall back
			}

			SetWindowPos( m_hWnd, HWND_TOPMOST, FullScreenRect.left, FullScreenRect.top, FullScreenRect.right, FullScreenRect.bottom, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			// force resize event
			SetMaximaized(true);
		}

		else
		{
			SetWindowLong(m_hWnd, GWL_STYLE, m_windowStyle);
			// force resize event
			SetMaximaized(false);
		}

	}

	void Window::ToggleBorderlessFullScreen()
	{
		SetBorderlessFullScreen(!IsBorderlessFullScreen());
	}

	void Window::Show()
	{
		ShowWindow( m_hWnd, SW_SHOW );
	}

	void Window::Hide()
	{
		ShowWindow( m_hWnd, SW_HIDE );
	}

	void Window::HandleResizeMessage(WPARAM wParam, LPARAM lParam )
	{
		IntPoint Size = IntPoint((int)(short)LOWORD( lParam ), (int)(short)HIWORD( lParam ));
		SizeChanged(Size);

		WORD resType = (WORD)wParam;
		//std::cout << resType << " - " << Size.ToString() << "\n";

		//if (resType == SIZE_MAXIMIZED)
		//{
		//}
		//
		//else if (resType == SIZE_RESTORED)
		//{
		//}
		//
		//else
		//{
		//}
	}

	void Window::SizeChanged( const IntPoint& NewSize )
	{
		m_WindowSize = NewSize;
		OnWindowResize.Braodcast(NewSize);
	}

	void Window::KeyPress( WPARAM Key, LPARAM lParam )
	{
		OnKeyPress.Braodcast(Key, lParam);
	}

	LRESULT CALLBACK Window::DefaultWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
#if WITH_EDITOR
		if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
			return true;
#endif

		// TODO: make a map of registered windows
		Window* W = nullptr;

		switch ( message )
		{
			//case WM_PAINT:
			//	break;
			//
			//
			//case WM_SYSKEYUP:
			//case WM_KEYUP:
			//	break;

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
				W = (Window*)GetWindowLongPtrA( hwnd, GWLP_USERDATA );
				W->KeyPress(wParam, lParam);
				return true;

			case WM_SIZE:
				W = (Window*)GetWindowLongPtrA( hwnd, GWLP_USERDATA );
				W->HandleResizeMessage(wParam, lParam);
				return true;

			case WM_CLOSE:
				W = (Window*)GetWindowLongPtrA( hwnd, GWLP_USERDATA );
				W->m_Closing = true;
				W->Hide();
				return true;

			default:
				return DefWindowProcW( hwnd, message, wParam, lParam );
		}
	}

}