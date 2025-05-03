#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"

namespace Drn
{
	class Texture : public Asset
	{
	public:

		Texture(const std::string& InPath)
			: Asset(InPath)
		{
		}

		virtual ~Texture()
		{
		}

protected:
		EAssetType GetAssetType() override = 0;

#if WITH_EDITOR
		void OpenAssetPreview() override = 0;
		void CloseAssetPreview() override = 0;
#endif
	};
}