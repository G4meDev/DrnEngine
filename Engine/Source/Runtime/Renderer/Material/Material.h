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

		void UploadResources( ID3D12GraphicsCommandList2* CommandList );
		void BindMainPass( ID3D12GraphicsCommandList2* CommandList );
		void BindPointLightShadowDepthPass( ID3D12GraphicsCommandList2* CommandList );
		void BindEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList );
		void BindSelectionPass( ID3D12GraphicsCommandList2* CommandList );
		void BindHitProxyPass( ID3D12GraphicsCommandList2* CommandList );

		void BindResources( ID3D12GraphicsCommandList2* CommandList );

		void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset);
		void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset);

		void SetIndexedScalar(uint32 Index, float Value);
		void SetIndexedVector(uint32 Index, const Vector4& Value);
		
		void SetNamedScalar(const std::string& Name, float Value);
		void SetNamedVector4(const std::string& Name, const Vector4& Value);


		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }

		inline bool IsSupportingMainPass() const { return m_SupportMainPass; }
		inline bool IsSupportingShadowPass() const { return m_SupportShadowPass; }
		inline bool IsSupportingHitProxyPass() const { return m_SupportHitProxyPass; }
		inline bool IsSupportingEditorPrimitivePass() const { return m_SupportEditorPrimitivePass; }
		inline bool IsSupportingEditorSelectionPass() const { return m_SupportEditorSelectionPass; }

		// TODO: remove
		bool m_BindlessTest = false;

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
		ShaderBlob m_EditorPrimitiveShaderBlob;
		ShaderBlob m_PointlightShadowDepthShaderBlob;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveType;
		EInputLayoutType m_InputLayoutType;
		D3D12_CULL_MODE m_CullMode;

		std::vector<Texture2DProperty> m_Texture2DSlots;
		std::vector<MaterialIndexedFloatParameter> m_FloatSlots;
		std::vector<MaterialIndexedVector4Parameter> m_Vector4Slots;

		Resource* m_ScalarBuffer;
		Resource* m_VectorBuffer;

		std::vector<float> m_ScalarValues;
		std::vector<Vector4> m_VectorValues;

		D3D12_CPU_DESCRIPTOR_HANDLE m_ScalarCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ScalarGpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE m_VectorCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_VectorGpuHandle;

		PipelineStateObject* m_MainPassPSO;
		PipelineStateObject* m_PointLightShadowDepthPassPSO;

#if WITH_EDITOR
		PipelineStateObject* m_SelectionPassPSO = nullptr;
		PipelineStateObject* m_HitProxyPassPSO = nullptr;
		PipelineStateObject* m_EditorProxyPSO = nullptr;
#endif

		std::unordered_map<std::string, MaterialIndexedFloatParameter*> m_ScalarMap;
		std::unordered_map<std::string, MaterialIndexedVector4Parameter*> m_Vector4Map;

		bool m_RenderStateDirty;

		bool m_SupportMainPass;
		bool m_SupportShadowPass;
		bool m_SupportHitProxyPass;
		bool m_SupportEditorPrimitivePass;
		bool m_SupportEditorSelectionPass;

		Resource* m_TextureIndexBuffer;

		D3D12_CPU_DESCRIPTOR_HANDLE m_TextureIndexCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_TextureIndexGpuHandle;

		void ReleaseBuffers();

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