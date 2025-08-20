#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include <DirectXTex.h>

LOG_DECLARE_CATEGORY(LogAssetImporterTexture2D);

namespace Drn
{
	class AssetImporterTexture
	{
	public:
		static void Import(Texture2D* TextureAsset, const std::string& Path);
		static void ImportDDS(Texture2D* TextureAsset, const std::string& Path);
		static void Import(TextureCube* TextureAsset, const std::string& Path);

	private:
		static void ImportTextureCubeFromTexture2D(TextureCube* TextureAsset, TexMetadata& MetaData, ScratchImage& Image);
	};
}

#endif