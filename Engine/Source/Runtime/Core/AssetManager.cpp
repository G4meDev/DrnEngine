#include "DrnPCH.h"
#include "AssetManager.h"

LOG_DEFINE_CATEGORY( LogAssetManager, "AssetManager" )

namespace Drn
{
	AssetManager* AssetManager::m_SingletionInstance;

	void AssetManager::Init()
	{
		m_SingletionInstance = new AssetManager();
	}

	void AssetManager::Shutdown()
	{
		if (m_SingletionInstance)
		{
			m_SingletionInstance->ReportLiveAssets();

			delete m_SingletionInstance;
			m_SingletionInstance = nullptr;
		}

	}

	void AssetManager::ReportLiveAssets()
	{
		OutputDebugString("Asset Manager: reporting live assets.\n");
		LOG(LogAssetManager, Info, "reprting live assets.");

		for (auto& it : m_AssetRegistery)
		{
			char Msg[500];
			sprintf(Msg, "\t(%u) %s\n", it.second->m_RefCount, it.first.c_str());
			OutputDebugString( Msg );

			LOG(LogAssetManager, Info, "\t(%u) %s", it.second->m_RefCount, it.first.c_str());
		}
	}

#if WITH_EDITOR
	void AssetManager::Create( const std::string& SourceFile, const std::string& TargetFolder )
	{
		std::string TargetFolderFullPath = Path::ConvertProjectPath(TargetFolder);

		if ( !FileSystem::DirectoryExists( TargetFolderFullPath ) )
		{
			LOG( LogAssetManager, Error, "selected folder in content browser doesnt exist on disk.\n\t%s ", TargetFolderFullPath.c_str() );
			return;
		}

		std::string FileName      = Path::ConvertShortPath( SourceFile );
		std::string FileExtension = Path::GetFileExtension( FileName );

		FileName                        = Path::RemoveFileExtension( FileName );
		FileName                        = Path::AddAssetFileExtension( FileName );
		const std::string AssetFilePath = TargetFolder + "\\" + FileName;

		LOG( LogAssetManager, Info, "trying to import file as\n\t%s ", AssetFilePath.c_str() );

		if ( FileSystem::DirectoryExists( AssetFilePath ) )
		{
			LOG( LogAssetManager, Error, "file already exists. try reimporting them. " );
			return;
		}

		std::shared_ptr<Asset> CreatedAsset;
		bool                          FormatSupported = false;

		if ( FileExtension == ".obj" || FileExtension == ".fbx" )
		{
			FormatSupported = true;
			CreatedAsset    = std::shared_ptr<Asset>(new StaticMesh( AssetFilePath, SourceFile ) );
		}

		else if ( FileExtension == ".hlsl" )
		{
			FormatSupported = true;
			CreatedAsset    = std::shared_ptr<Asset>(new Material( AssetFilePath, SourceFile ) );
		}

		if ( !FormatSupported )
		{
				LOG( LogAssetManager, Error, "file format %s is not supported. ", FileExtension.c_str() );
				return;
		}

		if ( CreatedAsset )
		{
				LOG( LogAssetManager, Info, "import succesful. " );
		}

		else
		{
				LOG( LogAssetManager, Error, "import failed. " );
		}
	}
#endif

}