#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "AssetPreview.h"

namespace Drn
{
	class AssetPreviewStaticMeshGuiLayer;
	class AssetImporterStaticMesh;

	class AssetPreviewStaticMesh : public AssetPreview
	{
	public:
		AssetPreviewStaticMesh(const std::string InPath);
		AssetPreviewStaticMesh(const std::string& InPath, const std::string& InSourcePath);
		virtual ~AssetPreviewStaticMesh();

		virtual void SetCurrentFocus() override;
		virtual void Reimport() override;

		virtual EAssetType GetAssetType() override;

		virtual void Serialize(Archive& Ar) override;

		virtual void Save() override;

	protected:

		StaticMeshData MeshData;

		float ImportScale = 1.0f;

		bool  ImportNormal = false;
		bool  ImportTangent = false;
		bool  ImportBiTangent = false;
		bool  ImportColor = true;
		uint8 ImportUvCount = 0;
		

		std::unique_ptr<AssetPreviewStaticMeshGuiLayer> GuiLayer;

		friend AssetPreviewStaticMeshGuiLayer;
		friend AssetImporterStaticMesh;

	private:
	};
}

#endif