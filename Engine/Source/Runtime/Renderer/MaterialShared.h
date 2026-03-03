#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/NamedProperty.h"

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

		inline void CopyParameters(const MaterialUniformParameters& Source)
		{
			m_Texture2DSlots = Source.m_Texture2DSlots;
			m_TextureCubeSlots = Source.m_TextureCubeSlots;
			m_FloatSlots = Source.m_FloatSlots;
			m_Vector4Slots = Source.m_Vector4Slots;
		};

		// only updates params with same name
		void OverrideParams(MaterialUniformParameters& Source);

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

// -------------------------------------------------------------------------------------------------------------------

	enum class EMaterialDomain : uint8
	{
		Surface,
		Decal
	};

	enum class VertexFactoryType : uint8
	{
		StaticMesh,
		InstancedStaticMesh,
	};

	struct MaterialShaderParameters
	{
		EMaterialDomain MaterialDomain;
		//EMaterialShadingModel ShadingModel;
		//EBlendMode BlendMode;

		union
		{
			uint32 PackedFlags;
			struct
			{
				uint32 bIsMasked : 1;
				uint32 bIsTwoSided : 1;

				uint32 bHasPrepass : 1;
				uint32 bHasCustomPrepass : 1;
				uint32 bHasShadowPass : 1;
				uint32 bHasMainPass : 1;
				uint32 bHasHitProxyPass : 1;
				uint32 bHasEditorPrimitivePass : 1;
				uint32 bHasEditorSelectionPass : 1;

				uint32 bIsUsedWithInstancedStaticMesh : 1;
			};
		};

		MaterialShaderParameters()
			: MaterialDomain(EMaterialDomain::Surface)
			, PackedFlags(0)
		{}

		MaterialShaderParameters(const class MaterialInterface* InMaterial)
		{
			//MaterialDomain = InMaterial->GetMaterialDomain();
			//ShadingModels = InMaterial->GetShadingModels();
			//BlendMode = InMaterial->GetBlendMode();

			//bIsMasked = InMaterial->IsMasked();
			//bIsMasked = InMaterial->IsTwoSided();
			//bIsMasked = InMaterial->IsUsedWithInstancedStaticMesh();
		}

		friend Archive& operator<<(Archive& Ar, const MaterialShaderParameters& Value)
		{
			Ar << (uint8)Value.MaterialDomain;
			Ar << Value.PackedFlags;
			return Ar;
		}

		friend Archive& operator>>(Archive& Ar, MaterialShaderParameters& Value)
		{
			Ar >> *(uint8*)&Value.MaterialDomain;
			Ar >> Value.PackedFlags;
			return Ar;
		}
	};
}