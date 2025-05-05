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
	{
		RECT WindowRect = { 0, 0, static_cast<LONG>( WindowSize.X ), static_cast<LONG>( WindowSize.Y) };
		AdjustWindowRect( &WindowRect, WS_OVERLAPPEDWINDOW, FALSE );

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

		SetWindowTitle(WindowName);
		SetFullscreen(false);
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

	void Window::SetFullscreen( bool FullScreen )
	{
		
	}

	void Window::ToggleFullScreen()
	{
		
	}

	void Window::Show()
	{
		ShowWindow( m_hWnd, SW_SHOW );
	}

	void Window::Hide()
	{
		ShowWindow( m_hWnd, SW_HIDE );
	}

	LRESULT CALLBACK Window::DefaultWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
#if WITH_EDITOR
		if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
			return true;
#endif

		switch ( message )
		{
			case WM_PAINT:
				break;

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
				break;

			case WM_SYSKEYUP:
			case WM_KEYUP:
				break;



			default:
				return DefWindowProcW( hwnd, message, wParam, lParam );
		}

		return 0;
	}
}