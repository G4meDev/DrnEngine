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

	class Material : public Asset
	{
	public:
		Material(const std::string& InPath);
		virtual ~Material();

#if WITH_EDITOR
		Material(const std::string& InPath, const std::string& InSourcePath);
#endif

		inline ID3D12RootSignature* GetRootSignature() { return m_RootSignature; }

		void UploadResources( ID3D12GraphicsCommandList2* CommandList );
		void BindMainPass( ID3D12GraphicsCommandList2* CommandList );
		void BindEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList );
		void BindSelectionPass( ID3D12GraphicsCommandList2* CommandList );
		void BindHitProxyPass( ID3D12GraphicsCommandList2* CommandList );

		void BindResources( ID3D12GraphicsCommandList2* CommandList );

		void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset);
		void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset);

		void SetNamedScalar(const std::string& Name, float Value);
		void SetNamedVector4(const std::string& Name, const Vector4& Value);

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }

		inline bool IsSupportingHitProxyPass() const { return m_SupportHitProxyPass; }

	protected:
		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Material; }

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		void Import();
#endif

	private:

		void ReleaseShaderBlobs();
		void ReleasePSOs();

		std::string m_SourcePath;

		ShaderBlob m_MainShaderBlob;
		ShaderBlob m_HitProxyShaderBlob;

		ID3D12RootSignature* m_RootSignature;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveType;
		EInputLayoutType m_InputLayoutType;
		D3D12_CULL_MODE m_CullMode;

		std::vector<Texture2DProperty> m_Texture2DSlots;
		std::vector<MaterialIndexedFloatParameter> m_FloatSlots;
		std::vector<MaterialIndexedVector4Parameter> m_Vector4Slots;

		Resource* m_ScalarCBV;

		D3D12_CPU_DESCRIPTOR_HANDLE m_ScalarCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ScalarGpuHandle;

		PipelineStateObject* m_MainPassPSO;

#if WITH_EDITOR
		PipelineStateObject* m_SelectionPassPSO = nullptr;
		PipelineStateObject* m_HitProxyPassPSO = nullptr;
		PipelineStateObject* m_EditorProxyPSO = nullptr;
#endif

		std::unordered_map<std::string, MaterialIndexedFloatParameter*> m_ScalarMap;
		std::unordered_map<std::string, MaterialIndexedVector4Parameter*> m_Vector4Map;

		bool m_RenderStateDirty;

		bool m_SupportHitProxyPass;

		void InitalizeParameterMap();

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		AssetPreviewMaterialGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewMaterialGuiLayer;
		friend class AssetImporterMaterial;

		friend class AssetManager;
	};
}