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

	std::string Path::ConvertShortPath( const std::string& FullPath )
	{
		size_t LastSlash = FullPath.find_last_of( "\\" );
		if ( LastSlash > 0 )
		{
			return FullPath.substr( LastSlash + 1, -1 );
		}

		return NAME_NULL;
	}

	std::string Path::GetFileExtension( const std::string& Path )
	{
		size_t LastSlash = Path.find_last_of( "." );
		if ( LastSlash > 0 )
		{
			return Path.substr( LastSlash, -1 );
		}

		return NAME_NULL;
	}

	std::string Path::RemoveFileExtension( const std::string& Path )
	{
		size_t LastSlash = Path.find_last_of( "." );
		if ( LastSlash > 0 )
		{
			return Path.substr( 0, LastSlash );
		}

		return NAME_NULL;
	}

	std::string Path::AddAssetFileExtension( const std::string& Path )
	{
		return Path + ".drn";
	}

#endif
}