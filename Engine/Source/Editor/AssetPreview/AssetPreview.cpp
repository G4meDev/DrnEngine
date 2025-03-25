#include "DrnPCH.h"
#include "AssetPreview.h"

#if WITH_EDITOR

#include "AssetPreviewStaticMesh.h"

namespace Drn
{
	AssetPreview::AssetPreview(const std::string InPath)
		: m_Path(InPath)
	{
		
	}

	AssetPreview::~AssetPreview()
	{
		
	}

	AssetPreview* AssetPreview::Create( const std::string InPath )
	{
		return new AssetPreviewStaticMesh(InPath);
	}
}

#endif