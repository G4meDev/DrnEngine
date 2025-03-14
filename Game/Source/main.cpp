#include <iostream>
#include <windows.h>

#include "GameApplication.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow)
{
	AllocConsole();
	static std::ofstream conout("CONOUT$", std::ios::out);
	std::cout.rdbuf(conout.rdbuf());

	GameApplication App;
	App.Run(hInstance);
}