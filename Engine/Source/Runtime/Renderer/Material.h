#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Renderer/InputLayout.h"
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

		inline bool HasBasePass() const { return ShaderParameters.bHasMainPass; }
		inline bool HasCustomPrePass() const { return ShaderParameters.bHasCustomPrepass; }
		inline bool HasPrePass() const { return ShaderParameters.bHasPrepass; }
		inline bool HasShadowPass() const { return ShaderParameters.bHasShadowPass; }
		inline bool HasHitProxyPass() const { return ShaderParameters.bHasHitProxyPass; }
		inline bool HasEditorPrimitivePass() const { return ShaderParameters.bHasEditorPrimitivePass; }
		inline bool HasEditorSelectionPass() const { return ShaderParameters.bHasEditorSelectionPass; }
		inline bool IsSupportingDeferredDecalPass() const { return m_SupportDeferredDecalPass; }
		inline bool IsSupportingStaticMeshDecalPass() const { return m_SupportStaticMeshDecalPass; }

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Material; }

		inline D3D12_CULL_MODE GetCullMode() const { return m_TwoSided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK; }

		inline EMaterialDomain GetMaterialDomain() const { return m_MaterialDomain; }

		inline bool IsTwoSided() const { return m_TwoSided; }

		inline const MaterialUniformParameters& GetParameters() const { return MaterialParameters; }

// -------------------------------------------------------------------------------------------------------------
// ---------------------------------------- Material Interface -------------------------------------------------
// -------------------------------------------------------------------------------------------------------------

		virtual Material* GetMaterial() const override { return const_cast<Material*>(this); };
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

		bool m_SupportDeferredDecalPass;
		bool m_SupportStaticMeshDecalPass;

		MaterialShaderParameters ShaderParameters;

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