#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Renderer/MaterialShared.h"
#include "Runtime/Renderer/MaterialInterface.h"

namespace Drn
{
	class AssetPreviewMaterialInstanceGuiLayer;
	class D3D12CommandList;
	class GraphicsPipelineState;
	struct MaterialPipelines;

	class MaterialInstance: public Asset, public MaterialInterface
	{
	public:
		MaterialInstance(const std::string& InPath);
		virtual ~MaterialInstance();

#if WITH_EDITOR
		MaterialInstance(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual EAssetType GetAssetType() override { return EAssetType::MaterialInstance; };
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::MaterialInstance; }

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }

// -------------------------------------------------------------------------------------------------------------
// ---------------------------------------- Material Interface -------------------------------------------------
// -------------------------------------------------------------------------------------------------------------

		virtual const Material* GetMaterial() const override { return *Parent; };
		virtual bool IsDependent(MaterialInterface* OtherMaterial) const override;

		virtual void UploadResources( class D3D12CommandList* CommandList ) override;
		virtual void BindResources( D3D12CommandList* CommandList ) override;

		virtual void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset) override;
		virtual void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset) override;

		virtual void SetNamedTextureCube(const std::string& Name, AssetHandle<TextureCube> TextureAsset) override;
		virtual void SetIndexedTextureCube(uint8 Index, AssetHandle<TextureCube> TextureAsset) override;

		virtual void SetIndexedScalar(uint32 Index, float Value) override;
		virtual void SetIndexedVector(uint32 Index, const Vector4& Value) override;
		
		virtual void SetNamedScalar(const std::string& Name, float Value) override;
		virtual void SetNamedVector4(const std::string& Name, const Vector4& Value) override;

	protected:
		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		void Import();
#endif

		friend class Editor;

	private:

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		AssetPreviewMaterialInstanceGuiLayer* GuiLayer = nullptr;
#endif

		std::string m_SourcePath;
		AssetHandle<Material> Parent;

		MaterialUniformParameters MaterialParameters;

		bool m_RenderStateDirty;


		friend AssetPreviewMaterialInstanceGuiLayer;
		friend AssetImporterMaterial;
	};
}