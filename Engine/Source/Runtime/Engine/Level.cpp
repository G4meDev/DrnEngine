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
		
	}
#endif

	Level::~Level()
	{
		
	}

	void Level::Serialize( Archive& Ar )
	{
		
	}

	void Level::Load()
	{
		
	}

	void Level::Save()
	{
		
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