#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Path
	{
	public:
		static std::wstring ShaderFullPath(LPCWSTR ShortPath);

		static std::string GetProjectPath();

		static std::string ConvertProjectPath(const std::string& Path);
		static std::string ConvertShortPath(const std::string& FullPath);
		
		static std::string GetFileExtension(const std::string& Path);
		static std::string RemoveFileExtension(const std::string& Path);
		static std::string AddAssetFileExtension(const std::string& Path);
	};
}