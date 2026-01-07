#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Renderer/InputLayout.h"
#include "Runtime/Engine/NamedProperty.h"
#include "Runtime/Renderer/ShaderBlob.h"
#include "Runtime/Renderer/MaterialShared.h"
#include "Runtime/Renderer/MaterialInterface.h"

LOG_DECLARE_CATEGORY(LogMaterial);

namespace Drn
{
	class AssetPreviewMaterialGuiLayer;
	class D3D12CommandList;
	class GraphicsPipelineState;
	struct MaterialPipelines;

	enum class EMaterialDomain : uint8
	{
		Surface,
		Decal
	};

	class Material : public Asset, public MaterialInterface
	{
	public:
		Material(const std::string& InPath);
		virtual ~Material();

#if WITH_EDITOR
		Material(const std::string& InPath, const std::string& InSourcePath);
#endif

		void BindMainPass( D3D12CommandList* CommandList );
		void BindPrePass( D3D12CommandList* CommandList );
		void BindPointLightShadowDepthPass( D3D12CommandList* CommandList );
		void BindSpotLightShadowDepthPass( D3D12CommandList* CommandList );
		void BindEditorPrimitivePass( D3D12CommandList* CommandList );
		void BindSelectionPass( D3D12CommandList* CommandList );
		void BindHitProxyPass( D3D12CommandList* CommandList );
		void BindDeferredDecalPass( D3D12CommandList* CommandList );
		void BindStaticMeshDecalPass( D3D12CommandList* CommandList );

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }

		inline bool IsSupportingBasePass() const { return m_SupportMainPass; }
		inline bool IsSupportingPrePass() const { return m_SupportPrePass; }
		inline bool IsSupportingShadowPass() const { return m_SupportShadowPass; }
		inline bool IsSupportingHitProxyPass() const { return m_SupportHitProxyPass; }
		inline bool IsSupportingEditorPrimitivePass() const { return m_SupportEditorPrimitivePass; }
		inline bool IsSupportingEditorSelectionPass() const { return m_SupportEditorSelectionPass; }
		inline bool IsSupportingDeferredDecalPass() const { return m_SupportDeferredDecalPass; }
		inline bool IsSupportingStaticMeshDecalPass() const { return m_SupportStaticMeshDecalPass; }

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Material; }

		inline D3D12_CULL_MODE GetCullMode() const { return m_TwoSided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK; }

		inline EMaterialDomain GetMaterialDomain() const { return m_MaterialDomain; }

		inline bool IsTwoSided() const { return m_TwoSided; }

// -------------------------------------------------------------------------------------------------------------
// ---------------------------------------- Material Interface -------------------------------------------------
// -------------------------------------------------------------------------------------------------------------

		virtual const Material* GetMaterial() const override { return this; };
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

		void ReleaseShaderBlobs();
		void ReleasePSOs();

		std::string m_SourcePath;

		ShaderBlob m_MainShaderBlob;
		ShaderBlob m_PrePassShaderBlob;
		ShaderBlob m_HitProxyShaderBlob;
		ShaderBlob m_EditorPrimitiveShaderBlob;
		ShaderBlob m_PointlightShadowDepthShaderBlob;
		ShaderBlob m_SpotlightShadowDepthShaderBlob;
		ShaderBlob m_DeferredDecalShaderBlob;
		ShaderBlob m_StaticMeshDecalShaderBlob;

		EMaterialDomain m_MaterialDomain;
		bool m_TwoSided;

		TRefCountPtr<MaterialPipelines> m_MaterialPipelines;
		MaterialUniformParameters MaterialParameters;

		bool m_RenderStateDirty;

		bool m_SupportMainPass;
		bool m_SupportPrePass;
		bool m_HasCustomPrePass;
		bool m_SupportShadowPass;
		bool m_SupportHitProxyPass;
		bool m_SupportEditorPrimitivePass;
		bool m_SupportEditorSelectionPass;
		bool m_SupportDeferredDecalPass;
		bool m_SupportStaticMeshDecalPass;

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		AssetPreviewMaterialGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewMaterialGuiLayer;
		friend class AssetImporterMaterial;

		friend class AssetManager;

		friend class MaterialPipelines;
		friend class StaticMeshSceneProxy;
	};
}