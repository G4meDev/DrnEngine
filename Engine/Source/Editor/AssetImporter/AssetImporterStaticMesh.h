#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogStaticMeshImporter);

namespace Drn
{
	class AssetPreviewStaticMesh;

	class AssetImporterStaticMesh
	{
	public:

		static void Import(AssetPreviewStaticMesh* MeshAsset, const std::string& Path);

	private:

		static void ProcessMesh(AssetPreviewStaticMesh* MeshAsset, aiMesh* mesh, const aiScene *scene);

	};
}

#endif