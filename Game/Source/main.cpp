#include <iostream>
#include <windows.h>

#include "GameApplication.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow){
	GameApplication App;
	return App.Run(hInstance);
}