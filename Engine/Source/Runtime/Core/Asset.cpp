#include "DrnPCH.h"
#include "Asset.h"

LOG_DEFINE_CATEGORY( LogAsset, "Asset" )

namespace Drn
{
	Asset::Asset(const std::string InPath)
		: m_Path(InPath)
		, m_RefCount(0)
	{
		
	}

	void Asset::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> m_AssetType;
		}

#if WITH_EDITOR

		else
		{
			uint16 AssetType = static_cast<uint16>(GetAssetType());
			Ar << AssetType;
		}

#endif
	}

	void Asset::Load()
	{
		FileArchive Ar(Path::ConvertProjectPath(m_Path));
		Serialize(Ar);
	}

#if WITH_EDITOR

	void Asset::Save()
	{
		FileArchive Ar(Path::ConvertProjectPath(m_Path), false);
		Serialize(Ar);
	}

#endif
}