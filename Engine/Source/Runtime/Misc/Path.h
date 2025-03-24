#pragma once

#include <string>

namespace Drn
{
	class Path
	{
	public:
		static std::wstring ShaderFullPath(LPCWSTR ShortPath);

#ifdef _DEBUG
		static std::string GetContentPath();

		static std::string ConvertFullPath(const std::string& ShortPath);
#endif
	};
}