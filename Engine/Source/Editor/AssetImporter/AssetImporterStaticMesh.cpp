#include "DrnPCH.h"
#include "AssetImporterStaticMesh.h"

#if WITH_EDITOR

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

LOG_DEFINE_CATEGORY( LogStaticMeshImporter, "StaticMeshImporter" );

namespace Drn
{
	void AssetImporterStaticMesh::Import( StaticMesh* MeshAsset, const std::string& Path )
	{
		ImportedStaticMeshData Data;
		
		if (!FileSystem::FileExists(Path))
		{
			LOG(LogStaticMeshImporter, Error, "source not found.\n importing failed.");
			return;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile( Path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace );

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
		{
			LOG(LogStaticMeshImporter, Error, "%s\nimporting failed.", importer.GetErrorString());
			return;
		}

		MeshAsset->m_BodySetup.m_AggGeo.EmptyElements();

		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			ProcessMesh( MeshAsset, scene->mMeshes[i], scene, Data);
		}

		Build(MeshAsset, Data);
	}

	void AssetImporterStaticMesh::ProcessMesh( StaticMesh* MeshAsset, aiMesh* mesh, const aiScene* scene, ImportedStaticMeshData& BuildingData)
	{
		std::string Name( mesh->mName.C_Str() );
		uint32 i = Name.find_first_of("_");
		std::string Prefix = Name.substr(0, i);

		if ( Prefix == "CS" )
		{
			ProcessCollisionSphere(MeshAsset, mesh, scene);
			return;
		}

		else if ( Prefix == "CB" )
		{
			ProcessCollisionBox(MeshAsset, mesh, scene);
			return;
		}

		else if ( Prefix == "CC" )
		{
			ProcessCollisionCapsule(MeshAsset, mesh, scene);
			return;
		}

		//LOG(LogStaticMeshImporter, Info, "%s", Prefix.c_str());

		ImportedStaticMeshSlotData MeshData;

		std::vector<InputLayout_StaticMesh> VertexData;
		std::vector<uint32> indices;

		for ( uint32 i = 0; i < mesh->mNumVertices; i++ )
		{
			InputLayout_StaticMesh Vertex;

			Vertex.X = mesh->mVertices[i].x * MeshAsset->ImportScale;
			Vertex.Y = mesh->mVertices[i].y * MeshAsset->ImportScale;
			Vertex.Z = mesh->mVertices[i].z * MeshAsset->ImportScale;

			Vertex.N_X = mesh->HasNormals() ? mesh->mNormals[i].x : 0;
			Vertex.N_Y = mesh->HasNormals() ? mesh->mNormals[i].y : 0;
			Vertex.N_Z = mesh->HasNormals() ? mesh->mNormals[i].z : 1;
			
			Vertex.T_X = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i].x : 0;
			Vertex.T_Y = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i].y : 0;
			Vertex.T_Z = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i].z : 1;
			
			Vertex.BT_X = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i].x : 0;
			Vertex.BT_Y = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i].y : 0;
			Vertex.BT_Z = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i].z : 1;

			Vertex.R = mesh->HasVertexColors(0) ? mesh->mColors[0][i].r : 1;
			Vertex.G = mesh->HasVertexColors(0) ? mesh->mColors[0][i].g : 1;
			Vertex.B = mesh->HasVertexColors(0) ? mesh->mColors[0][i].b : 1;

			Vertex.U1 = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i].x : 0;
			Vertex.V1 = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i].y : 0;

			Vertex.U2 = mesh->HasTextureCoords(1) ? mesh->mTextureCoords[1][i].x : 0;
			Vertex.V2 = mesh->HasTextureCoords(1) ? mesh->mTextureCoords[1][i].y : 0;

			Vertex.U3 = mesh->HasTextureCoords(2) ? mesh->mTextureCoords[2][i].x : 0;
			Vertex.V3 = mesh->HasTextureCoords(2) ? mesh->mTextureCoords[2][i].y : 0;

			Vertex.U4 = mesh->HasTextureCoords(3) ? mesh->mTextureCoords[3][i].x : 0;
			Vertex.V4 = mesh->HasTextureCoords(3) ? mesh->mTextureCoords[3][i].y : 0;

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
			std::string MatName = material->GetName().C_Str();
			MeshData.MaterialIndex = BuildingData.AddMaterial(MatName);
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

	void AssetImporterStaticMesh::Build( StaticMesh* MeshAsset, ImportedStaticMeshData& BuildingData) 
	{
		std::vector<MaterialData> OldMaterials;
		
		for (const MaterialData& Mat : MeshAsset->Data.Materials)
		{
			OldMaterials.push_back({});
			OldMaterials[OldMaterials.size() - 1].m_Name = Mat.m_Name;
			OldMaterials[OldMaterials.size() - 1].m_Material = Mat.m_Material;
		}

		MeshAsset->Data.MeshesData.clear();
		MeshAsset->Data.Materials.clear();

		MeshAsset->Data.MeshesData.resize(BuildingData.MeshesData.size());
		for (int i = 0; i < BuildingData.MeshesData.size(); i++)
		{
			ImportedStaticMeshSlotData& IMD = BuildingData.MeshesData[i];
			StaticMeshSlotData& Data = MeshAsset->Data.MeshesData[i];

			uint32 VertexByteCount = IMD.Vertices.size() * sizeof( InputLayout_StaticMesh );
			D3DCreateBlob(VertexByteCount, &Data.VertexBufferBlob);
			memcpy(Data.VertexBufferBlob->GetBufferPointer(), IMD.Vertices.data(), VertexByteCount);

			uint32 IndexByteCount = IMD.Indices.size() * sizeof(uint32);
			D3DCreateBlob(IndexByteCount, &Data.IndexBufferBlob);
			memcpy(Data.IndexBufferBlob->GetBufferPointer(), IMD.Indices.data(), IndexByteCount);

			Data.MaterialIndex = IMD.MaterialIndex;
		}

		MeshAsset->Data.Materials.resize(BuildingData.MaterialsData.size());
		for (int i = 0; i < BuildingData.MaterialsData.size(); i++)
		{
			const std::string& MatName = BuildingData.MaterialsData[i];

			MaterialData& M = MeshAsset->Data.Materials[i]; 
			M.m_Name = MatName;
			
			std::string MaterialPath = "";
			
			for (MaterialData& OldMaterial : OldMaterials)
			{
				if (OldMaterial.m_Name == MatName)
				{
					MaterialPath = OldMaterial.m_Material.GetPath();
				}
			}
			
			MaterialPath = MaterialPath != "" ? MaterialPath : DEFAULT_MATERIAL_PATH;
			M.m_Material = AssetHandle<Material>(MaterialPath);
			M.m_Material.Load();
		}

// -------------------------------------------------------------------------------------------------------

		for (int i = 0; i < BuildingData.MeshesData.size(); i++)
		{
			ImportedStaticMeshSlotData& IMD = BuildingData.MeshesData[i];
			StaticMeshSlotData& Data = MeshAsset->Data.MeshesData[i];

			uint64 VertexCount = IMD.Vertices.size();
			Data.Positions.resize(VertexCount);
			Data.Normals.resize(VertexCount);
			Data.Tangents.resize(VertexCount);
			Data.BitTangents.resize(VertexCount);

			for (uint64 i = 0; i < VertexCount; i++)
			{
				Data.Positions[i] = IMD.Vertices[i].GetPosition();
				Data.Normals[i] = IMD.Vertices[i].GetNormal();
				Data.Tangents[i] = IMD.Vertices[i].GetTangent();
				Data.BitTangents[i] = IMD.Vertices[i].GetBitTangent();
			}
		}
	}


	void AssetImporterStaticMesh::ProcessCollisionSphere( StaticMesh* MeshAsset, aiMesh* mesh, const aiScene* scene )
	{
		if (mesh->mNumVertices <= 0)
		{
			return;
		}

		LOG(LogStaticMeshImporter, Info, "%i", mesh->mNumVertices);

		Vector Pos = Vector::ZeroVector;

		for ( uint32 i = 0; i < mesh->mNumVertices; i++ )
		{
			float Pos_X = mesh->mVertices[i].x * MeshAsset->ImportScale;
			float Pos_Y = mesh->mVertices[i].y * MeshAsset->ImportScale;
			float Pos_Z = mesh->mVertices[i].z * MeshAsset->ImportScale;

			Pos = Pos + Vector(Pos_X, Pos_Y, Pos_Z);
		}

		float Pos_X = mesh->mVertices[0].x * MeshAsset->ImportScale;
		float Pos_Y = mesh->mVertices[0].y * MeshAsset->ImportScale;
		float Pos_Z = mesh->mVertices[0].z * MeshAsset->ImportScale;
		Vector Pos_0 = Vector(Pos_X, Pos_Y, Pos_Z);

		Vector Center = Pos / mesh->mNumVertices;
		float Radius = Vector::Distance(Center, Pos_0);

		MeshAsset->m_BodySetup.m_AggGeo.SphereElems.push_back(SphereElem(Radius, Center));
	}

	void AssetImporterStaticMesh::ProcessCollisionBox( StaticMesh* MeshAsset, aiMesh* mesh, const aiScene* scene )
	{
		MeshAsset->m_BodySetup.m_AggGeo.BoxElems.push_back(BoxElem(Vector::ZeroVector, Quat::Identity, Vector::OneVector * Vector(1, 1, 1)));
	}

	void AssetImporterStaticMesh::ProcessCollisionCapsule( StaticMesh* MeshAsset, aiMesh* mesh, const aiScene* scene )
	{
		
	}

}
#endif