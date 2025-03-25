#include "DrnPCH.h"
#include "AssetPreview.h"

#if WITH_EDITOR

#include "AssetPreviewStaticMesh.h"

LOG_DEFINE_CATEGORY( LogAssetPreview, "AssetPreview" );

namespace Drn
{
	AssetPreview::AssetPreview(const std::string InPath)
		: m_Path(InPath)
	{

	}

	AssetPreview::AssetPreview( const std::string& InPath, const std::string InSourcePath )
		: m_Path(InSourcePath)
		, m_SourcePath(InSourcePath)
	{
		
	}

	AssetPreview::~AssetPreview()
	{
		
	}

	std::shared_ptr<AssetPreview> AssetPreview::Open( const std::string InPath )
	{
		return std::shared_ptr<AssetPreviewStaticMesh>(new AssetPreviewStaticMesh(InPath));
	}

	void AssetPreview::Create(const std::string& SourceFilePath, const std::string& TargetDirectoryPath)
	{
		if (!FileSystem::DirectoryExists(TargetDirectoryPath))
		{
			LOG( LogAssetPreview, Error, "selected folder in content browser doesnt exist on disk.\n\t%s ", TargetDirectoryPath.c_str());
			return;
		}

		std::string FileName = Path::ConvertShortPath(SourceFilePath);
		std::string FileExtension = Path::GetFileExtension(FileName);
		
		FileName = Path::RemoveFileExtension(FileName);
		FileName = Path::AddAssetFileExtension(FileName);
		const std::string AssetFilePath = TargetDirectoryPath + "\\" + FileName;

		LOG( LogAssetPreview, Info, "trying to import file as\n\t%s ", AssetFilePath.c_str() );

		if (FileSystem::DirectoryExists(AssetFilePath))
		{
			LOG( LogAssetPreview, Error, "file already exists. try reimporting them. ");
			return;
		}

		std::shared_ptr<AssetPreview> CreatedAsset;
		bool FormatSupported = false;

		if ( FileExtension == ".obj" )
		{
			FormatSupported = true;
			CreatedAsset = std::shared_ptr<AssetPreview>(new AssetPreviewStaticMesh(AssetFilePath, SourceFilePath));
		}

		if (!FormatSupported)
		{
			LOG( LogAssetPreview, Error, "file format %s is not supported. ", FileExtension.c_str());
			return;
		}

		if (CreatedAsset)
		{
			LOG( LogAssetPreview, Info, "import succesful. " );
		}

		else
		{
			LOG( LogAssetPreview, Error, "import failed. " );
		}
	}

	void AssetPreview::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			uint8 byte;

			Ar >> byte;
			Ar >> m_SourcePath;
		}

		else
		{
			uint8 type = static_cast<uint8>(GetAssetType());
			Ar << type << m_SourcePath << type << type;
		}
	}

}

#endif