#include <iostream>
#include <windows.h>

#include "GameApplication.h"
#include <corecrt_io.h>
#include <fcntl.h>

#define MAX_CONSOLE_LINES 500;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow)
{
	if ( AllocConsole() )
	{
		HANDLE lStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );

		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo( lStdHandle, &consoleInfo );
		consoleInfo.dwSize.Y = MAX_CONSOLE_LINES;
		SetConsoleScreenBufferSize( lStdHandle, consoleInfo.dwSize );
		SetConsoleCursorPosition( lStdHandle, { 0, 0 } );

		int   hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
		FILE* fp         = _fdopen( hConHandle, "w" );
		freopen_s( &fp, "CONOUT$", "w", stdout );
		setvbuf( stdout, nullptr, _IONBF, 0 );

		lStdHandle = GetStdHandle( STD_INPUT_HANDLE );
		hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
		fp         = _fdopen( hConHandle, "r" );
		freopen_s( &fp, "CONIN$", "r", stdin );
		setvbuf( stdin, nullptr, _IONBF, 0 );

		lStdHandle = GetStdHandle( STD_ERROR_HANDLE );
		hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
		fp         = _fdopen( hConHandle, "w" );
		freopen_s( &fp, "CONOUT$", "w", stderr );
		setvbuf( stderr, nullptr, _IONBF, 0 );

		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();
	}

	GameApplication App;
	return App.Run(hInstance);
}