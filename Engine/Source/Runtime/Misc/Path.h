#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY( LogPath );

namespace Drn
{
	class Path
	{
	public:
		static std::wstring ShaderFullPath(LPCWSTR ShortPath);

		static std::string GetProjectPath();

		static std::string ConvertProjectPath(const std::string& Path);
		static std::string ConvertShortPath(const std::string& FullPath);
		static std::string GetDirectory(const std::string& FullPath);
		
		static std::string GetCleanName(const std::string& FullPath);

		static std::string GetFileExtension(const std::string& Path);
		static std::string RemoveFileExtension(const std::string& Path);
		static std::string AddAssetFileExtension(const std::string& Path);

		static std::string GetNewTempArchivePath();

		static std::string GetIntemediatePath();
		static std::string GetThumbnailPath();

		static bool GetNameForNewAsset( const std::string& TargetDirectory, const std::string& NamePrefix, std::string& Result);
	};
}