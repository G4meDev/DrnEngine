#include "DrnPCH.h"
#include "AssetImporterStaticMesh.h"

#if WITH_EDITOR

#include "Editor/AssetPreview/AssetPreviewStaticMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

LOG_DEFINE_CATEGORY( LogStaticMeshImporter, "StaticMeshImporter" );

namespace Drn
{
	void AssetImporterStaticMesh::Import( AssetPreviewStaticMesh* MeshAsset, const std::string& Path )
	{
		ImportedStaticMeshData Data;
		
		if (!FileSystem::FileExists(Path))
		{
			LOG(LogStaticMeshImporter, Error, "source not found.\n importing failed.");
			return;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile( Path, aiProcess_Triangulate | aiProcess_FlipUVs );

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
		{
			LOG(LogStaticMeshImporter, Error, "%s\nimporting failed.", importer.GetErrorString());
			return;
		}

		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			ProcessMesh( MeshAsset, scene->mMeshes[i], scene, Data);
		}

		Build(MeshAsset, Data);
	}

	void AssetImporterStaticMesh::ProcessMesh( AssetPreviewStaticMesh* MeshAsset, aiMesh* mesh, const aiScene* scene, ImportedStaticMeshData& BuildingData)
	{
		ImportedStaticMeshSlotData MeshData;

		std::vector<StaticMeshVertexData> VertexData;
		std::vector<uint32> indices;

		for ( uint32 i = 0; i < mesh->mNumVertices; i++ )
		{
			StaticMeshVertexData Vertex;
			
			Vertex.Pos_X = mesh->mVertices[i].x;
			Vertex.Pos_Y = mesh->mVertices[i].y;
			Vertex.Pos_Z = mesh->mVertices[i].z;

			Vertex.Normal_X = mesh->HasNormals() ? mesh->mNormals[i].x : 0;
			Vertex.Normal_Y = mesh->HasNormals() ? mesh->mNormals[i].y : 0;
			Vertex.Normal_Z = mesh->HasNormals() ? mesh->mNormals[i].z : 1;
			
			Vertex.Tangent_X = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i].x : 0;
			Vertex.Tangent_Y = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i].y : 0;
			Vertex.Tangent_Z = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i].z : 1;
			
			Vertex.BiTangent_X = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i].x : 0;
			Vertex.BiTangent_Y = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i].y : 0;
			Vertex.BiTangent_Z = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i].z : 1;

			Vertex.Color_R = mesh->HasVertexColors(0) ? mesh->mColors[0][i].r : 1;
			Vertex.Color_G = mesh->HasVertexColors(0) ? mesh->mColors[0][i].g : 1;
			Vertex.Color_B = mesh->HasVertexColors(0) ? mesh->mColors[0][i].b : 1;

			Vertex.U_1 = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i].x : 0;
			Vertex.V_1 = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i].y : 0;

			Vertex.U_2 = mesh->HasTextureCoords(1) ? mesh->mTextureCoords[1][i].x : 0;
			Vertex.V_2 = mesh->HasTextureCoords(1) ? mesh->mTextureCoords[1][i].y : 0;

			Vertex.U_3 = mesh->HasTextureCoords(2) ? mesh->mTextureCoords[2][i].x : 0;
			Vertex.V_3 = mesh->HasTextureCoords(2) ? mesh->mTextureCoords[2][i].y : 0;

			Vertex.U_4 = mesh->HasTextureCoords(3) ? mesh->mTextureCoords[3][i].x : 0;
			Vertex.V_4 = mesh->HasTextureCoords(3) ? mesh->mTextureCoords[3][i].y : 0;

			VertexData.push_back( Vertex );
		}

		MeshData.Vertices = VertexData;

		for ( uint32 i = 0; i < mesh->mNumFaces; i++ )
		{
			aiFace face = mesh->mFaces[i];
			for ( uint32 j = 0; j < face.mNumIndices; j++ )
				indices.push_back( face.mIndices[j] );
		}

		MeshData.Indices = indices;

		if ( mesh->mMaterialIndex >= 0 )
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			MeshData.MaterialIndex = BuildingData.AddMaterial(std::string( material->GetName().C_Str()));
		}

		BuildingData.MeshesData.push_back(MeshData);
	}

	uint8 ImportedStaticMeshData::AddMaterial( std::string& InMaterial )
	{
		for ( int i = 0; i < MaterialsData.size(); i++ )
		{
			if ( MaterialsData[i] == InMaterial)
			{
				return i;
			}
		}

		MaterialsData.push_back( InMaterial );
		return MaterialsData.size() - 1;
	}

	void AssetImporterStaticMesh::Build( AssetPreviewStaticMesh* MeshAsset, ImportedStaticMeshData& BuildingData) 
	{
		MeshAsset->MeshData.MeshesData.clear();
		MeshAsset->MeshData.Materials.clear();

		for (ImportedStaticMeshSlotData& Mesh : BuildingData.MeshesData)
		{
			StaticMeshSlotData Data;
			StaticMeshVertexBuffer VertexBuffer;

			for (auto& Vertex : Mesh.Vertices)
			{
				VertexBuffer.Pos_X = Vertex.Pos_X;
				VertexBuffer.Pos_Y = Vertex.Pos_Y;
				VertexBuffer.Pos_Z = Vertex.Pos_Z;
				
				VertexBuffer.Color_R = Vertex.Color_R;
				VertexBuffer.Color_G = Vertex.Color_G;
				VertexBuffer.Color_B = Vertex.Color_B;

				Data.VertexData.push_back(VertexBuffer);
			}

			Data.IndexData = Mesh.Indices;

			Data.Stride = 1;
			Data.MaterialIndex = Mesh.MaterialIndex;

			MeshAsset->MeshData.MeshesData.push_back(Data);
		}

		MeshAsset->MeshData.Materials = BuildingData.MaterialsData;
	}

}

#endif