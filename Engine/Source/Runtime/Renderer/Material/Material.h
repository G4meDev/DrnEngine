#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"

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


	protected:
		virtual EAssetType GetAssetType() override;
		virtual void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		void Import();
#endif

	private:
		std::string m_SourcePath;

		ID3DBlob* m_VS_Blob;
		ID3DBlob* m_PS_Blob;
		ID3DBlob* m_GS_Blob;
		ID3DBlob* m_HS_Blob;
		ID3DBlob* m_DS_Blob;
		ID3DBlob* m_CS_Blob;

		void ReleaseShaderBlobs();

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		AssetPreviewMaterialGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewMaterialGuiLayer;
		friend class AssetImporterMaterial;
	};
}