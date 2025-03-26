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

		MeshAsset->MeshesData.clear();
		MeshAsset->MaterialsData.clear();

		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			ProcessMesh( MeshAsset, scene->mMeshes[i], scene );
		}

	}


	void AssetImporterStaticMesh::ProcessMesh( AssetPreviewStaticMesh* MeshAsset, aiMesh* mesh, const aiScene* scene )
	{
		StaticMeshData MeshData;

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

			MaterialData Data;
			Data.Name = material->GetName().C_Str();

			MeshData.MaterialIndex = MeshAsset->AddMaterial(Data);
		}

		MeshAsset->MeshesData.push_back(MeshData);
	}

}

#endif