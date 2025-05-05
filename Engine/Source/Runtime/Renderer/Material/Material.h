#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Renderer/InputLayout.h"

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

		inline ID3DBlob* GetVS() const { return m_VS_Blob; }
		inline ID3DBlob* GetPS() const { return m_PS_Blob; }
		inline ID3DBlob* GetGS() const { return m_GS_Blob; }
		inline ID3DBlob* GetHS() const { return m_HS_Blob; }
		inline ID3DBlob* GetDS() const { return m_DS_Blob; }
		inline ID3DBlob* GetCS() const { return m_CS_Blob; }

		inline ID3D12RootSignature* GetRootSignature() { return m_RootSignature; }
		inline ID3D12PipelineState* GetBasePassPSO() { return m_BasePassPSO; }

		void UploadResources( ID3D12GraphicsCommandList2* CommandList );
		inline bool IsLoadedOnGpu() const { return m_LoadedOnGPU; }

	protected:
		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Material; }

		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		void Import();
#endif

	private:

		void ReleaseShaderBlobs();

		std::string m_SourcePath;

		ID3DBlob* m_VS_Blob;
		ID3DBlob* m_PS_Blob;
		ID3DBlob* m_GS_Blob;
		ID3DBlob* m_HS_Blob;
		ID3DBlob* m_DS_Blob;

		// TODO: maybe remove this?
		ID3DBlob* m_CS_Blob;

		ID3D12RootSignature* m_RootSignature;
		ID3D12PipelineState* m_BasePassPSO;
		bool m_LoadedOnGPU;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveType;
		EInputLayoutType m_InputLayoutType;
		D3D12_CULL_MODE m_CullMode;

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