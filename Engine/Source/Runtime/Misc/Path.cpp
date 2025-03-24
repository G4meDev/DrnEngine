#include "DrnPCH.h"
#include "Path.h"

#define SHADER_PATH std::wstring(L"../Engine/Source/Shaders/")

#define PATH_CONTENT "..\\..\\..\\Engine\\Content"

namespace Drn
{
	std::wstring Path::ShaderFullPath(LPCWSTR ShortPath)
	{
		return SHADER_PATH + ShortPath;
	}

#if _DEBUG

	std::string Path::GetContentPath()
	{
		return PATH_CONTENT;
	}

	std::string Path::ConvertFullPath(const std::string& ShortPath)
	{
		return std::string(PATH_CONTENT) + "\\" + ShortPath;
	}

#endif
}