#include "DrnPCH.h"
#include "Asset.h"

LOG_DEFINE_CATEGORY( LogAsset, "Asset" )

namespace Drn
{
	Asset::Asset(const std::string InPath)
		: m_Path(InPath)
		, RefCount(0)
	{
		
	}

	Asset::Asset( const std::string InPath, const std::string InSourcePath )
		: m_Path(InPath)
		, m_SourcePath(InSourcePath)
		, RefCount(0)
	{
		
	}
}