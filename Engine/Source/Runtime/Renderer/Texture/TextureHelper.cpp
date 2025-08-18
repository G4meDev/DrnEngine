#include "DrnPCH.h"
#include "TextureHelper.h"

namespace Drn
{
	std::string TextureHelper::GetDxgiFormatName( DXGI_FORMAT Foramt )
	{
		switch (Foramt)
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return "R8G8B8A8_UNORM_SRGB";

			case DXGI_FORMAT_R32G32B32A32_FLOAT: return "R32G32B32A32_FLOAT";

			case DXGI_FORMAT_BC1_UNORM: return "BC1_UNORM";
			case DXGI_FORMAT_BC1_UNORM_SRGB: return "BC1_UNORM_SRGB";

			case DXGI_FORMAT_BC4_UNORM: return "BC4_UNORM";
			case DXGI_FORMAT_BC5_UNORM: return "BC5_UNORM";
			case DXGI_FORMAT_BC6H_UF16: return "BC6H_UF16";

			default: return "Unkown Format";
		}
	}


	DXGI_FORMAT TextureHelper::ResolveTextureFormat( DXGI_FORMAT InFormat, ETextureCompression Compression)
	{
		if (InFormat == DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			if (Compression == ETextureCompression::BC1)
			{
				return DXGI_FORMAT_BC1_UNORM;
			}

			else if (Compression == ETextureCompression::BC4)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}

			else if (Compression == ETextureCompression::BC5)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
		}

		else if (InFormat == DXGI_FORMAT_R32G32B32A32_FLOAT)
		{
			if (Compression == ETextureCompression::BC1)
			{
				return DXGI_FORMAT_BC1_UNORM;
			}

			else if (Compression == ETextureCompression::BC6)
			{
				return DXGI_FORMAT_BC6H_UF16;
			}
		}

		return InFormat;
	}


}