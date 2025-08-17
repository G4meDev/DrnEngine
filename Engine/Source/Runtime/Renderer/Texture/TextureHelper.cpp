#include "DrnPCH.h"
#include "TextureHelper.h"

namespace Drn
{
	std::string TextureHelper::GetDxgiFormatName( DXGI_FORMAT Foramt )
	{
		if (Foramt == DXGI_FORMAT_R32G32B32A32_FLOAT)
		{
			return "RGBA32_Float";
		}

		else
		{
			return "Unkown Format";
		}
	}

}