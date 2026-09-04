#pragma once

#include "ForwardTypes.h"

#if WITH_EDITOR

#include "Runtime/Engine/SkeletalMeshVertexData.h"

LOG_DECLARE_CATEGORY( LogSkeletalMeshImporter );

namespace Drn
{
	struct ImportedSkeletalMeshSlotData
	{
	public:
		ImportedSkeletalMeshSlotData() {};

		SkeletalMeshVertexData VertexData;
		uint8 MaterialIndex;
		Box Bound;
	};

	struct ImportedSkeletalMeshData
	{
	public:
		ImportedSkeletalMeshData(){};

		uint8 AddMaterial(std::string& InMaterial);

		std::vector<ImportedSkeletalMeshSlotData> MeshesData;
		std::vector<std::string> MaterialsData;

		ReferenceSkeleton RefSkeleton;

		//std::unordered_map<class aiNode*, MeshBoneInfo> BoneMap;
	};

	class AssetImporterSkeletalMesh
	{
	public:
		static void Import(SkeletalMesh* MeshAsset, const std::string& Path);

	private:
		static void ProcessSkeleton(SkeletalMesh* MeshAsset, const aiScene *scene, ImportedSkeletalMeshData& BuildingData);
		static void ProcessSkeleton(int32 ParentIndex, const aiNode *node, ImportedSkeletalMeshData& BuildingData);
		static void ProcessMesh(SkeletalMesh* MeshAsset, aiMesh* mesh, const aiScene *scene, ImportedSkeletalMeshData& BuildingData);
		static void Build(SkeletalMesh* MeshAsset, ImportedSkeletalMeshData& BuildingData);
	};
}

#endif