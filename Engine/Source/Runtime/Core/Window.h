#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogWindow);
#define DEFAULT_WINDOW_CLASS_NAME L"DefaultWindowClass"

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam(OnWindowResizeDelegate, const IntPoint&);
	DECLARE_MULTICAST_DELEGATE_TwoParams(OnKeyPressDelegate, WPARAM, LPARAM);

	class Window
	{
	public:

		Window( HINSTANCE hInstance, const std::wstring& ClassName, const std::wstring& WindowName, const IntPoint& WindowSize );
		~Window();

		static void RegisterDefaultClass( HINSTANCE hInstance );

		inline bool IsClosing() const { return m_Closing; }
		inline void SetClosing() { m_Closing = true; }

		inline HWND GetWindowHandle() const { return m_hWnd; }
		inline const std::wstring& GetWindowName() const { return m_Name; }

		inline const std::wstring& GetWindowTitle() const { return m_Title; }
		void SetWindowTitle( const std::wstring& windowTitle );

		inline const IntPoint& GetWindowSize() const { return m_WindowSize; }
		void SetWindowSize( const IntPoint& windowTitle );

		bool IsMaximaized() const;
		void SetMaximaized(bool bMaximazied);
		void ToggleMaximaized();

		bool IsBorderlessFullScreen() const;
		void SetBorderlessFullScreen(bool bBorderless);
		void ToggleBorderlessFullScreen();

		void Show();
		void Hide();

		static LRESULT CALLBACK DefaultWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

		OnWindowResizeDelegate OnWindowResize;
		OnKeyPressDelegate OnKeyPress;

	private:

		void HandleResizeMessage(WPARAM wParam, LPARAM lParam);

		void SizeChanged(const IntPoint& NewSize);
		void KeyPress(WPARAM Key, LPARAM lParam);

		HWND m_hWnd;

		std::wstring m_Name;
		std::wstring m_Title;
		IntPoint m_WindowSize;

		bool m_Borderless;
		bool m_Closing;

		static const UINT m_windowStyle = WS_OVERLAPPEDWINDOW;
	};
}