#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class TextureHelper
	{
	public:
		static std::string GetDxgiFormatName(DXGI_FORMAT Foramt);
		static DXGI_FORMAT ResolveTextureFormat(DXGI_FORMAT InFormat, ETextureCompression Compression);
	};
}