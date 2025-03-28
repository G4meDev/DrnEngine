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

}