#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogStaticMeshImporter);

namespace Drn
{
	class AssetPreviewStaticMesh;

	struct StaticMeshVertexData
	{
	public:

		float Pos_X;
		float Pos_Y;
		float Pos_Z;

		float Normal_X;
		float Normal_Y;
		float Normal_Z;

		float Tangent_X;
		float Tangent_Y;
		float Tangent_Z;

		float BiTangent_X;
		float BiTangent_Y;
		float BiTangent_Z;

		float Color_R;
		float Color_G;
		float Color_B;

		float U_1;
		float V_1;
		
		float U_2;
		float V_2;

		float U_3;
		float V_3;

		float U_4;
		float V_4;
	};

	struct ImportedStaticMeshSlotData
	{
	public:
		ImportedStaticMeshSlotData() {};

		std::vector<StaticMeshVertexData> Vertices;
		std::vector<uint32> Indices;

		uint8 MaterialIndex;
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

		static void Build(StaticMesh* MeshAsset, ImportedStaticMeshData& BuildingData);
	};
}

#endif