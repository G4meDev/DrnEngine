#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Path
	{
	public:
		static std::wstring ShaderFullPath(LPCWSTR ShortPath);

#ifdef _DEBUG
		static std::string GetContentPath();

		static std::string ConvertFullPath(const std::string& ShortPath);
		static std::string ConvertShortPath(const std::string& FullPath);
		
		static std::string GetFileExtension(const std::string& Path);
		static std::string RemoveFileExtension(const std::string& Path);
		static std::string AddAssetFileExtension(const std::string& Path);

#endif
	};
}