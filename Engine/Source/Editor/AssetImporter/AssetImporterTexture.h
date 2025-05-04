#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogAssetImporterTexture2D);

namespace Drn
{
	class AssetImporterTexture
	{
	public:
		static void Import(Texture2D* TextureAsset, const std::string& Path);
	};
}

#endif