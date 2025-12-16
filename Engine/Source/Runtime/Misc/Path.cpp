#include "DrnPCH.h"
#include "Path.h"

#define SHADER_PATH std::wstring(L"../Engine/Source/Shaders/")

#define PROJECT_PATH "..\\..\\.."
#define INTEMEDIATE_PATH "..\\..\\..\\Intermediate\\"
#define THUMBNAIL_PATH "..\\..\\..\\Intermediate\\Thumbnail\\"

LOG_DEFINE_CATEGORY( LogPath, "LogPath" );

namespace Drn
{
	std::wstring Path::ShaderFullPath(LPCWSTR ShortPath)
	{
		return SHADER_PATH + ShortPath;
	}


	std::string Path::GetProjectPath()
	{
		return PROJECT_PATH;
	}

	std::string Path::ConvertProjectPath( const std::string& Path )
	{
		return std::string(PROJECT_PATH) + "\\" + Path;
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

	std::string Path::GetDirectory( const std::string& FullPath )
	{
		size_t found_f = FullPath.find_last_of('/');
		size_t found_b = FullPath.find_last_of('\\');
		size_t last_slash_idx = std::string::npos;

		if (found_f != std::string::npos && found_b != std::string::npos)
		{
			last_slash_idx = std::max(found_f, found_b);
		}
		else if (found_f != std::string::npos)
		{
			last_slash_idx = found_f;
		}
		else if (found_b != std::string::npos)
		{
			last_slash_idx = found_b;
		}

		if (last_slash_idx != std::string::npos) {
			return FullPath.substr(0, last_slash_idx);
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

	std::string Path::GetNewTempArchivePath()
	{
		return INTEMEDIATE_PATH + Guid::NewGuid().ToString();
	}

	std::string Path::GetIntemediatePath()
	{
		return INTEMEDIATE_PATH;
	}

	std::string Path::GetThumbnailPath()
	{
		return THUMBNAIL_PATH;
	}

	bool Path::GetNameForNewAsset( const std::string& TargetDirectory, const std::string& NamePrefix, std::string& Result )
	{
		if ( !FileSystem::DirectoryExists( TargetDirectory ) )
		{
			LOG( LogPath, Error, "Selected directory for new asset doesnt exist.\n\t%s ", TargetDirectory.c_str() );
			return false;
		}

		std::unique_ptr<SystemFileNode> Root;

		// @TODO: make a non recursive version
		FileSystem::GetFilesInDirectory(TargetDirectory, Root, ".drn");

		for (int ix = 1; ix < 100; ix++)
		{
			std::stringstream ss;
			ss << std::setw(2) << std::setfill('0') << ix;
			std::string TargetName = NamePrefix + ss.str() + ".drn";

			bool NameCollision = false;
			for (SystemFileNode* Child : Root->Childs)
			{
				if (!Child->File.m_IsDirectory && Child->File.m_ShortPath == TargetName)
				{
					NameCollision = true;
				}
			}

			if (!NameCollision)
			{
				Result = TargetName;
				return true;
			}
		}

		return false;
	}

}