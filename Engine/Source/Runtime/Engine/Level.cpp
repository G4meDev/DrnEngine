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
		: Asset(InPath)
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
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			
		}

#if WITH_EDITOR
		else
		{

		}
#endif
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