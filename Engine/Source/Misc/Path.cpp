#include "DrnPCH.h"
#include "Path.h"

#define SHADER_PATH std::wstring(L"../Engine/Source/Shaders/")

namespace Drn
{
	std::wstring Path::ShaderFullPath(LPCWSTR ShortPath)
	{
		return SHADER_PATH + ShortPath;
	}

}