#pragma once

#include "ForwardTypes.h"

#if WITH_EDITOR

LOG_DECLARE_CATEGORY( LogMeshImporterHelper );

namespace Drn
{
	enum class EMeshImporterPreviewFlags : uint32
	{
		None				= 1 << 0,
		HasStaticMeshes		= 1 << 0,
		HasSkeletalMeshes	= 1 << 1,
		IsValidSource		= 1 << 2,
	};

	class MeshImporterHelper
	{
	public:
		static EMeshImporterPreviewFlags PreviewMeshSource(const std::string& SourcePath);
	};
}

#endif