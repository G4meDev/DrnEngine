#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/NamedProperty.h"
#include "Runtime/Renderer/VertexDeclaration.h"

#define VERTEX_FACTORY_NAME_NULL "NullVertexFactory"

namespace Drn
{
	//DECLARE_DELEGATE_RetVal( VertexDeclaration* , VertexDeclarationDelegate )

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

	class VertexFactoryType
	{
	public:
		VertexFactoryType(std::string&& InName, TRefCountPtr<VertexDeclaration>* InVertexDeclaration, TRefCountPtr<VertexDeclaration>* InVertexDeclarationDefaultDepthOnly)
			: Name(std::move(InName))
			, VDeclaration(InVertexDeclaration)
			, VDeclarationDefaultDepthOnly(InVertexDeclarationDefaultDepthOnly)
		{
			drn_check(std::find_if(GlobalFactories.begin(), GlobalFactories.end(),
				[&](const VertexFactoryType* Factroy){ return Factroy->GetName() == Name; }) == GlobalFactories.end());

			GlobalFactories.push_back(this);
		}

		inline const std::string& GetName() const { return Name; }

		inline static VertexFactoryType* GetVertexFactory(const std::string Name)
		{
			if (Name == VERTEX_FACTORY_NAME_NULL)
			{
				return nullptr;
			}

			auto It = std::find_if(GlobalFactories.begin(), GlobalFactories.end(),
				[&](const VertexFactoryType* Factroy){ return Factroy->GetName() == Name; });

			drn_check(It != GlobalFactories.end());
			return *It;
		}

		inline VertexDeclaration* GetVertexDeclaration() const
		{
			drn_check(VDeclaration);
			return *VDeclaration;
		}

		inline VertexDeclaration* GetVertexDeclarationDefaultDepthOnly() const
		{
			return VDeclarationDefaultDepthOnly ? *VDeclarationDefaultDepthOnly : nullptr;
		}

		static std::vector<VertexFactoryType*> GlobalFactories;

		static VertexFactoryType* StaticMesh;
		static VertexFactoryType* Decal;

	private:
		const std::string Name;
		TRefCountPtr<VertexDeclaration>* VDeclaration;
		TRefCountPtr<VertexDeclaration>* VDeclarationDefaultDepthOnly;

		// @TODO: maybe add hashed name for faster comparison
	};

// -------------------------------------------------------------------------------------------------------------------

	enum class EMaterialDomain : uint8
	{
		Surface,
		Decal
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
				uint32 bHasDecalPass : 1;
				uint32 bHasStaticMeshDecalPass : 1;
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

#if WITH_EDITOR
		void Draw();
#endif
	};

// -------------------------------------------------------------------------------------------------------------------

	enum class EMaterialStage : uint8
	{
		Prepass,
		PointLightShadow,
		SpotLightShadow,
		Main,
		Decal,
		Hitproxy,
		EditorPrimitive,
		EditorSelection,
		StaticMeshDecal,
	};

	class MaterialShader
	{
	public:
		MaterialShader()
			: VertexFactory(nullptr)
			, MaterialStage(EMaterialStage::Main)
			, Blob()
			, PipelineState(nullptr)
		{}

		MaterialShader(VertexFactoryType* InVertexFactory, EMaterialStage InMaterialStage, const ShaderBlob& InBlob)
			: VertexFactory(InVertexFactory)
			, MaterialStage(InMaterialStage)
			, Blob(InBlob)
			, PipelineState(nullptr)
		{}

		inline bool CheckTypeStage(VertexFactoryType* InVertexFactory, EMaterialStage InMaterialStage) const
		{
			return (GetVertexFactoryType() == InVertexFactory) && (GetMaterialStage() == InMaterialStage);
		}

		inline bool operator==( const MaterialShader& Other ) const
		{
			return CheckTypeStage(Other.GetVertexFactoryType(), Other.GetMaterialStage());
		}

		inline VertexFactoryType* GetVertexFactoryType() const { return VertexFactory; }
		inline EMaterialStage GetMaterialStage() const { return MaterialStage; }

		inline friend Archive& operator<<(Archive& Ar, MaterialShader& Value)
		{
			Ar << (Value.VertexFactory ? Value.VertexFactory->GetName() : VERTEX_FACTORY_NAME_NULL);
			Ar << static_cast<uint8>(Value.MaterialStage);
			Value.Blob.Serialize(Ar);

			return Ar;
		}

		inline friend Archive& operator>>(Archive& Ar, MaterialShader& Value)
		{
			std::string VertexFactoryName;
			Ar >> VertexFactoryName;
			Value.VertexFactory = VertexFactoryType::GetVertexFactory(VertexFactoryName);

			Ar >> *(uint8*)&Value.MaterialStage;
			Value.Blob.Serialize(Ar);

			return Ar;
		}

		void UploadPipelineState(class D3D12CommandList* CmdList, Material* InMaterial);
		void Bind(class D3D12CommandList* CmdList);
		inline void SetPipeline(TRefCountPtr<class GraphicsPipelineState> InPipeline) { PipelineState = InPipeline; }

	private:
		VertexFactoryType* VertexFactory;
		EMaterialStage MaterialStage;
		ShaderBlob Blob;

		TRefCountPtr<class GraphicsPipelineState> PipelineState;
	};

	class MaterialShaders
	{
	public:
		MaterialShaders() {}

		inline void PushShader(VertexFactoryType* InVertexFactory, EMaterialStage InMaterialStage, const ShaderBlob& InBlob)
		{
			Shaders.push_back(MaterialShader(InVertexFactory, InMaterialStage, InBlob));
		}

		inline MaterialShader* GetShader(VertexFactoryType* InVertexFactory, EMaterialStage InMaterialStage) const
		{
			auto It = std::find_if(Shaders.begin(), Shaders.end(), [&](const MaterialShader& InMaterialShader)
				{ return InMaterialShader.CheckTypeStage(InVertexFactory, InMaterialStage); });

			return It._Ptr;
		}

		inline friend Archive& operator<<(Archive& Ar, MaterialShaders& Value)
		{
			uint64 ShaderCount = Value.Shaders.size();
			drn_check(ShaderCount < UINT8_MAX);
			Ar << static_cast<uint8>(ShaderCount);

			for (MaterialShader& MatShader : Value.Shaders)
			{
				Ar << MatShader;
			}

			return Ar;
		}

		inline friend Archive& operator>>(Archive& Ar, MaterialShaders& Value)
		{
			uint8 ShaderCount;
			Ar >> ShaderCount;

			Value.Shaders.clear();
			Value.Shaders.resize(ShaderCount);
			for (MaterialShader& MatShader : Value.Shaders)
			{
				Ar >> MatShader;
			}

			return Ar;
		}

		void UploadPipelineStates(class D3D12CommandList* CmdList, Material* InMaterial);

	private:
		std::vector<MaterialShader> Shaders;
	};

	const wchar_t* GetVertexFactoryShaderMacro( VertexFactoryType* VertexFactory );

// -------------------------------------------------------------------------------------------------------------------


}