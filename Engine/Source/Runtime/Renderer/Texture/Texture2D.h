#pragma once

#include "ForwardTypes.h"
#include "Texture.h"

namespace Drn
{
	class Texture2D : public Texture
	{
	public:
		Texture2D(const std::string& InPath);
#if WITH_EDITOR
		Texture2D(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual ~Texture2D();


	protected:
		EAssetType GetAssetType() override { return EAssetType::Texture2D; };
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Texture2D; }

#if WITH_EDITOR
		void OpenAssetPreview() override;
		void CloseAssetPreview() override;
#endif
	};
}