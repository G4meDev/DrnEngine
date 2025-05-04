#include "DrnPCH.h"
#include "AssetImporterTexture.h"

#if WITH_EDITOR

#include <DirectXTex.h>

LOG_DEFINE_CATEGORY( LogAssetImporterTexture2D, "AssetImporterTexture2D" );

namespace Drn
{
	void AssetImporterTexture::Import( Texture2D* TextureAsset, const std::string& Path )
	{
		std::string Extension = Path::GetFileExtension(Path);

		TexMetadata  metadata;
		ScratchImage scratchImage;
		HRESULT Result = E_FAIL;

		if ( Extension == ".tga" )
		{
			Result = LoadFromTGAFile(StringHelper::s2ws(Path).c_str(), &metadata, scratchImage);
		}

		if (Result == S_OK && metadata.dimension == TEX_DIMENSION_TEXTURE2D)
		{
			if (TextureAsset->IsSRGB())
			{
				metadata.format = MakeSRGB(metadata.format);
			}

			TextureAsset->m_SizeX = metadata.width;
			TextureAsset->m_SizeY = metadata.height;
			TextureAsset->m_MipLevels = metadata.mipLevels;
			TextureAsset->m_Format = metadata.format;

			// TODO: support mip importing later
			const DirectX::Image* BaseImage = scratchImage.GetImage(0, 0, 0);

			TextureAsset->ReleaseImageBlobs();
			const uint32 ImageBufferSize = BaseImage->slicePitch;
			D3DCreateBlob(ImageBufferSize, &TextureAsset->m_ImageBlob);
			memcpy(TextureAsset->m_ImageBlob->GetBufferPointer(), BaseImage->pixels, ImageBufferSize);

			TextureAsset->m_RowPitch = BaseImage->rowPitch;
			TextureAsset->m_SlicePitch = BaseImage->slicePitch;
		}

		else
		{
			LOG(LogAssetImporterTexture2D, Error, "faield to load texture2d from %s", Path.c_str());
		}
	}

}

#endif