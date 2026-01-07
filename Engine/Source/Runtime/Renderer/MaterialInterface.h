#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Material;

	class MaterialInterface
	{
	public:
		virtual Material* GetMaterial() const = 0;
		virtual bool IsDependent(MaterialInterface* OtherMaterial) const = 0;
		virtual bool IsTwoSided() const;

		virtual void UploadResources( class D3D12CommandList* CommandList ) = 0;
		virtual void BindResources( D3D12CommandList* CommandList ) = 0;

		virtual void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset) = 0;
		virtual void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset) = 0;

		virtual void SetNamedTextureCube(const std::string& Name, AssetHandle<TextureCube> TextureAsset) = 0;
		virtual void SetIndexedTextureCube(uint8 Index, AssetHandle<TextureCube> TextureAsset) = 0;

		virtual void SetIndexedScalar(uint32 Index, float Value) = 0;
		virtual void SetIndexedVector(uint32 Index, const Vector4& Value) = 0;
		
		virtual void SetNamedScalar(const std::string& Name, float Value) = 0;
		virtual void SetNamedVector4(const std::string& Name, const Vector4& Value) = 0;
	};
}