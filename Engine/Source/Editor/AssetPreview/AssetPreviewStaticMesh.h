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
		~AssetPreviewStaticMesh();

		virtual void SetCurrentFocus() override;
	protected:

		std::unique_ptr<AssetPreviewStaticMeshGuiLayer> GuiLayer;

	private:
	};
}

#endif