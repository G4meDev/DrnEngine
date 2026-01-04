#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class RenderUniformBuffer;

	struct MaterialUniformParameters : public Serializable
	{
		MaterialUniformParameters() {}

		virtual void Serialize(Archive& Ar) override;
		void UploadResources( class D3D12CommandList* CommandList );

		inline void Clear()
		{
			m_Texture2DSlots.clear();
			m_TextureCubeSlots.clear();
			m_FloatSlots.clear();
			m_Vector4Slots.clear();
		}

		void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset);
		void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset);

		void SetNamedTextureCube(const std::string& Name, AssetHandle<TextureCube> TextureAsset);
		void SetIndexedTextureCube(uint8 Index, AssetHandle<TextureCube> TextureAsset);

		void SetIndexedScalar(uint32 Index, float Value);
		void SetIndexedVector(uint32 Index, const Vector4& Value);
		
		void SetNamedScalar(const std::string& Name, float Value);
		void SetNamedVector4(const std::string& Name, const Vector4& Value);

		std::vector<Texture2DProperty> m_Texture2DSlots;
		std::vector<TextureCubeProperty> m_TextureCubeSlots;
		std::vector<FloatProperty> m_FloatSlots;
		std::vector<Vector4Property> m_Vector4Slots;

		TRefCountPtr<RenderUniformBuffer> ParametersBuffer;
	};
}