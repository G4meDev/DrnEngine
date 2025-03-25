#pragma once

#if WITH_EDITOR

#include "AssetPreview.h"

namespace Drn
{
	class AssetPreviewStaticMeshGuiLayer;

	class AssetPreviewStaticMesh : public AssetPreview
	{
	public:
		AssetPreviewStaticMesh(const std::string InPath);
		AssetPreviewStaticMesh(const std::string& InPath, const std::string& InSourcePath);
		virtual ~AssetPreviewStaticMesh();

		virtual void SetCurrentFocus() override;
		virtual void Reimport() override;

		virtual EAssetType GetAssetType() override;

	protected:

		std::unique_ptr<AssetPreviewStaticMeshGuiLayer> GuiLayer;

	private:
	};
}

#endif