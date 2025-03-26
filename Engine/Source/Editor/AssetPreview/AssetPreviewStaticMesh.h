#pragma once

#if WITH_EDITOR

#include "AssetPreview.h"

namespace Drn
{
	class AssetPreviewStaticMeshGuiLayer;
	class AssetImporterStaticMesh;

	struct StaticMeshVertexData
	{
	public:
		StaticMeshVertexData(){};

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
		float Color_B;
		float Color_G;

		float U_1;
		float V_1;
		
		float U_2;
		float V_2;

		float U_3;
		float V_3;

		float U_4;
		float V_4;
	};

	struct MaterialData
	{
	public:
		MaterialData(){};

		std::string Name;
	};

	struct StaticMeshData
	{
	public:
		StaticMeshData(){};

		std::vector<StaticMeshVertexData> Vertices;
		std::vector<uint32> Indices;

		uint8 MaterialIndex;
	};

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

		std::vector<StaticMeshData> MeshesData;
		std::vector<MaterialData> MaterialsData;

		uint8 AddMaterial(MaterialData Material);

		float ImportScale = 1.0f;

		std::unique_ptr<AssetPreviewStaticMeshGuiLayer> GuiLayer;
		
		friend AssetPreviewStaticMeshGuiLayer;
		friend AssetImporterStaticMesh;


	private:
	};
}

#endif