#include "DrnPCH.h"
#include "Level.h"

namespace Drn
{
	Level::Level( const std::string& Path )
		: Asset(Path)
	{
		
	}

#if WITH_EDITOR
	Level::Level( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath, InSourcePath)
	{
		Save();

		std::cout << InPath;
	}
#endif

	Level::~Level()
	{
		
	}

	void Level::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			
		}

#if WITH_EDITOR
		else
		{
			Ar << static_cast<uint8>(GetAssetType());
			Ar << m_AssetVersion;
			Ar << m_SourcePath;
		}
#endif
	}

	void Level::Load()
	{
		Archive Ar = Archive(Path::ConvertProjectPath(m_Path), true);
		Serialize(Ar);
	}

	void Level::Save()
	{
		Archive Ar = Archive(Path::ConvertProjectPath(m_Path), false);
		Serialize(Ar);
	}

	void Level::Import()
	{
		
	}

	EAssetType Level::GetAssetType()
	{
		return EAssetType::Level;
	}

	void Level::OpenAssetPreview()
	{
		
	}

	void Level::CloseAssetPreview()
	{
	
	}


 }