#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/InputLayout.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

LOG_DECLARE_CATEGORY(LogStaticMeshImporter);

namespace Drn
{
	class AssetPreviewStaticMesh;

	struct ImportedStaticMeshSlotData
	{
	public:
		ImportedStaticMeshSlotData() {};

		std::vector<InputLayout_StaticMesh> Vertices;

		std::vector<uint32> Indices;
		uint8 MaterialIndex;

		uint32 MaxIndex = 0;
	};

	struct ImportedStaticMeshData
	{
	public:
		ImportedStaticMeshData(){};

		uint8 AddMaterial(std::string& InMaterial);

		std::vector<ImportedStaticMeshSlotData> MeshesData;
		std::vector<std::string> MaterialsData;
	};

	class AssetImporterStaticMesh
	{
	public:

		static void Import(StaticMesh* MeshAsset, const std::string& Path);

	private:

		static void ProcessMesh(StaticMesh* MeshAsset, aiMesh* mesh, const aiScene *scene, ImportedStaticMeshData& BuildingData);
		static void ProcessCollisionSphere(StaticMesh* MeshAsset, aiMesh* mesh, const aiScene *scene);
		static void ProcessCollisionBox(StaticMesh* MeshAsset, aiMesh* mesh, const aiScene *scene);
		static void ProcessCollisionCapsule(StaticMesh* MeshAsset, aiMesh* mesh, const aiScene *scene);

		static void Build(StaticMesh* MeshAsset, ImportedStaticMeshData& BuildingData);
	};
}

#endif