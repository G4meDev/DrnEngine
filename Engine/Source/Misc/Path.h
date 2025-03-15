#pragma once

#include <string>

namespace Drn
{
	class Path
	{
	public:
		static std::wstring ShaderFullPath(LPCWSTR ShortPath);
	};
}