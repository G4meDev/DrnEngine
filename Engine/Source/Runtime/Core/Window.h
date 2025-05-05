#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogWindow);
#define DEFAULT_WINDOW_CLASS_NAME L"DefaultWindowClass"

namespace Drn
{

	class Window
	{
	public:

		using OnSizeChanged = std::function<void( const IntPoint& NewSize )>;
		using OnKeyPress = std::function<void( WPARAM )>;

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

		inline bool IsFullScreen() const { return m_FullScreen; }
		void SetFullscreen(bool FullScreen);
		void ToggleFullScreen();

		void Show();
		void Hide();

		static LRESULT CALLBACK DefaultWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

		void BindOnSizeChanged(OnSizeChanged Delegate);
		void ClearOnSizeChanged();
		void InvokeOnSizeChanged(const IntPoint& NewSize);

		OnSizeChanged OnSizeChangedDelegate;

		void BindOnKeyPress(OnKeyPress Delegate);
		void ClearOnKeyPress();
		void InvokeOnKeyPress(WPARAM Key);

		OnKeyPress OnKeyPressDelegate;

	private:

		void SizeChanged(const IntPoint& NewSize);
		void KeyPress(WPARAM Key);

		HWND m_hWnd;

		std::wstring m_Name;
		std::wstring m_Title;
		IntPoint m_WindowSize;
		bool m_FullScreen;

		bool m_Closing;
	};
}