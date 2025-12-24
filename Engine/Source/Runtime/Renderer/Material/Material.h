#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Renderer/InputLayout.h"
#include "Runtime/Engine/NamedProperty.h"
#include "Runtime/Renderer/ShaderBlob.h"

LOG_DECLARE_CATEGORY(LogMaterial);

namespace Drn
{
	class AssetPreviewMaterialGuiLayer;
	class D3D12CommandList;
	class GraphicsPipelineState;

	enum class EMaterialDomain : uint8
	{
		Surface,
		Decal
	};

	class Material : public Asset
	{
	public:
		Material(const std::string& InPath);
		virtual ~Material();

#if WITH_EDITOR
		Material(const std::string& InPath, const std::string& InSourcePath);
#endif

		void UploadResources( class D3D12CommandList* CommandList );
		void BindMainPass( D3D12CommandList* CommandList );
		void BindPrePass( D3D12CommandList* CommandList );
		void BindPointLightShadowDepthPass( D3D12CommandList* CommandList );
		void BindSpotLightShadowDepthPass( D3D12CommandList* CommandList );
		void BindEditorPrimitivePass( D3D12CommandList* CommandList );
		void BindSelectionPass( D3D12CommandList* CommandList );
		void BindHitProxyPass( D3D12CommandList* CommandList );
		void BindDeferredDecalPass( D3D12CommandList* CommandList );
		void BindStaticMeshDecalPass( D3D12CommandList* CommandList );

		void BindResources( ID3D12GraphicsCommandList2* CommandList );

		void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset);
		void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset);

		void SetNamedTextureCube(const std::string& Name, AssetHandle<TextureCube> TextureAsset);
		void SetIndexedTextureCube(uint8 Index, AssetHandle<TextureCube> TextureAsset);

		void SetIndexedScalar(uint32 Index, float Value);
		void SetIndexedVector(uint32 Index, const Vector4& Value);
		
		void SetNamedScalar(const std::string& Name, float Value);
		void SetNamedVector4(const std::string& Name, const Vector4& Value);


		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; m_TextureBufferDirty = true; m_ScalarBufferDirty = true; m_VectorBufferDirty = true; }
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

		std::vector<MaterialIndexedTexture2DParameter> m_Texture2DSlots;
		std::vector<MaterialIndexedTextureCubeParameter> m_TextureCubeSlots;
		std::vector<MaterialIndexedFloatParameter> m_FloatSlots;
		std::vector<MaterialIndexedVector4Parameter> m_Vector4Slots;

		TRefCountPtr<class RenderUniformBuffer> ScalarBuffer;
		TRefCountPtr<class RenderUniformBuffer> VectorBuffer;

		std::vector<float> m_ScalarValues;
		std::vector<Vector4> m_VectorValues;

		TRefCountPtr<GraphicsPipelineState> m_MainPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_PrePassPSO;
		TRefCountPtr<GraphicsPipelineState> m_PointLightShadowDepthPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_SpotLightShadowDepthPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_DeferredDecalPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_StaticMeshDecalPassPSO;

#if WITH_EDITOR
		TRefCountPtr<GraphicsPipelineState> m_SelectionPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_HitProxyPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_EditorProxyPSO;
#endif

		std::unordered_map<std::string, MaterialIndexedFloatParameter*> m_ScalarMap;
		std::unordered_map<std::string, MaterialIndexedVector4Parameter*> m_Vector4Map;

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

		bool m_ScalarBufferDirty;
		bool m_VectorBufferDirty;
		bool m_TextureBufferDirty;

		TRefCountPtr<class RenderUniformBuffer> TextureIndexBuffer;

		void InitalizeParameterMap();

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		AssetPreviewMaterialGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewMaterialGuiLayer;
		friend class AssetImporterMaterial;

		friend class AssetManager;

		// TODO: remove
		friend class StaticMeshSceneProxy;
	};
}