#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/MaterialInterface.h"

namespace Drn
{
	class MaterialInterface;
	class Texture2D;
	class TextureCube;

	class MaterialInstanceBase : public MaterialInterface
	{
	public:

		
	protected:
		MaterialInterface* Parent;

		//void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset);
		//void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset);
		//
		//void SetNamedTextureCube(const std::string& Name, AssetHandle<TextureCube> TextureAsset);
		//void SetIndexedTextureCube(uint8 Index, AssetHandle<TextureCube> TextureAsset);
		//
		//void SetNamedVector4(const std::string& Name, const Vector4& Value);
		//void SetIndexedVector(uint32 Index, const Vector4& Value);
		//
		//void SetNamedScalar(const std::string& Name, float Value);
		//void SetIndexedScalar(uint32 Index, float Value);

	};
}