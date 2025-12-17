#include "DrnPCH.h"
#include "AssetImporterStaticMesh.h"

#if WITH_EDITOR

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

LOG_DEFINE_CATEGORY( LogStaticMeshImporter, "StaticMeshImporter" );

#define MIN_SPHERE_BOUNDS 0.05f
#define MIN_BOX_BOUNDS 0.01f

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
		//const aiScene* scene = importer.ReadFile( Path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace );
		const aiScene* scene = importer.ReadFile( Path, aiProcess_Triangulate | aiProcess_FlipUVs );

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
		{
			LOG(LogStaticMeshImporter, Error, "%s\nimporting failed.", importer.GetErrorString());
			return;
		}

		MeshAsset->m_BodySetup.m_AggGeo.EmptyElements();
		MeshAsset->m_BodySetup.m_TriMeshes.clear();

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
			Vertex.A = mesh->HasVertexColors(0) ? mesh->mColors[0][i].a : 1;

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
			for (uint32 j = 0; j < face.mNumIndices; j++)
			{
				MeshData.Indices.push_back(face.mIndices[j]);
				MeshData.MaxIndex = std::max(MeshData.MaxIndex, MeshData.Indices.back());
			}
		}

		if ( mesh->mMaterialIndex >= 0 )
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::string MatName = material->GetName().C_Str();
			MeshData.MaterialIndex = BuildingData.AddMaterial(MatName);
		}

		BuildingData.MeshesData.push_back(MeshData);

		// --------------------------------------------------------------------------------------------

		BodySetup& MeshBodySetup = MeshAsset->m_BodySetup;
		MeshBodySetup.m_TriMeshes.push_back({});

		const uint32 VertexCount = MeshData.Vertices.size();
		std::vector<PxVec3> Positions;
		Positions.resize(VertexCount);
		for (uint32 i = 0; i < VertexCount; i++) { Positions[i] = Vector2P(MeshData.Vertices[i].GetPosition()); }

		PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = VertexCount;
		meshDesc.points.stride = sizeof(PxVec3);
		meshDesc.points.data = Positions.data();

		const uint32 IndexCount = MeshData.Indices.size();

		meshDesc.triangles.count = IndexCount / 3;
		meshDesc.triangles.stride = 3 * sizeof(uint32);
		meshDesc.triangles.data = MeshData.Indices.data();

		PxDefaultMemoryOutputStream WriteStream;
		PxTriangleMeshCookingResult::Enum result;
		physx::PxCookingParams CookParam = physx::PxCookingParams( physx::PxTolerancesScale());
		bool Success = PxCookTriangleMesh(CookParam, meshDesc, WriteStream, &result);

		std::vector<uint8>& CookData = MeshBodySetup.m_TriMeshes.back().CookData;
		CookData.resize(WriteStream.getSize());
		memcpy(CookData.data(), WriteStream.getData(), WriteStream.getSize());
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

		float MaxX = -FLT_MAX, MaxY = -FLT_MAX, MaxZ = -FLT_MAX;
		float MinX = FLT_MAX, MinY = FLT_MAX, MinZ = FLT_MAX;

		for (int i = 0; i < BuildingData.MeshesData.size(); i++)
		{
			ImportedStaticMeshSlotData& IMD = BuildingData.MeshesData[i];
			StaticMeshSlotData& Data = MeshAsset->Data.MeshesData[i];

			Data.MaterialIndex = IMD.MaterialIndex;

			{
				const bool ShouldUse4BitIndices = IMD.MaxIndex > UINT16_MAX;
				Data.VertexData.bUse4BitIndices = ShouldUse4BitIndices;

				if (ShouldUse4BitIndices)
				{
					Data.VertexData.Indices_32 = IMD.Indices;
				}

				else
				{
					Data.VertexData.Indices_16.clear();
					for (int32 i = 0; i < IMD.Indices.size(); i++)
					{
						Data.VertexData.Indices_16.push_back(IMD.Indices[i]);
					}
				}
			}

			{
				const uint64 VertexCount = IMD.Vertices.size();
				Data.VertexData.VertexCount = VertexCount;

				Data.VertexData.Positions.resize(VertexCount);
				Data.VertexData.Normals.resize(MeshAsset->m_ImportNormals ? VertexCount : 0);
				Data.VertexData.Tangents.resize(MeshAsset->m_ImportTangents ? VertexCount : 0);
				Data.VertexData.BitTangents.resize(MeshAsset->m_ImportBitTangents ? VertexCount : 0);
				Data.VertexData.Colors.resize(MeshAsset->m_ImportColor? VertexCount : 0);

				Data.VertexData.UV_1.resize(MeshAsset->m_ImportUVs >= 1 ? VertexCount : 0);
				Data.VertexData.UV_2.resize(MeshAsset->m_ImportUVs >= 2 ? VertexCount : 0);
				Data.VertexData.UV_3.resize(MeshAsset->m_ImportUVs >= 3 ? VertexCount : 0);
				Data.VertexData.UV_4.resize(MeshAsset->m_ImportUVs >= 4 ? VertexCount : 0);

				for (uint64 i = 0; i < VertexCount; i++)
				{
					Data.VertexData.Positions[i] = Vector(IMD.Vertices[i].X, IMD.Vertices[i].Y, IMD.Vertices[i].Z);

					if (MeshAsset->m_ImportNormals)
						Data.VertexData.Normals[i] = Math::PackSignedNormalizedVectorToUint32(Vector(IMD.Vertices[i].N_X, IMD.Vertices[i].N_Y, IMD.Vertices[i].N_Z));

					if (MeshAsset->m_ImportTangents)
						Data.VertexData.Tangents[i] = Math::PackSignedNormalizedVectorToUint32(Vector(IMD.Vertices[i].T_X, IMD.Vertices[i].T_Y, IMD.Vertices[i].T_Z));

					if (MeshAsset->m_ImportBitTangents)
						Data.VertexData.BitTangents[i] = Math::PackSignedNormalizedVectorToUint32(Vector(IMD.Vertices[i].BT_X, IMD.Vertices[i].BT_Y, IMD.Vertices[i].BT_Z));

					if (MeshAsset->m_ImportColor)
						Data.VertexData.Colors[i] = Vector4(IMD.Vertices[i].R, IMD.Vertices[i].G, IMD.Vertices[i].B, IMD.Vertices[i].A);

					if (MeshAsset->m_ImportUVs >= 1)
						Data.VertexData.UV_1[i] = Vector2Half(IMD.Vertices[i].U1, IMD.Vertices[i].V1);

					if (MeshAsset->m_ImportUVs >= 2)
						Data.VertexData.UV_2[i] = Vector2Half(IMD.Vertices[i].U2, IMD.Vertices[i].V2);

					if (MeshAsset->m_ImportUVs >= 3)
						Data.VertexData.UV_3[i] = Vector2Half(IMD.Vertices[i].U3, IMD.Vertices[i].V3);

					if (MeshAsset->m_ImportUVs >= 4)
						Data.VertexData.UV_4[i] = Vector2Half(IMD.Vertices[i].U4, IMD.Vertices[i].V4);

					MaxX = std::max(MaxX, Data.VertexData.Positions[i].GetX());
					MaxY = std::max(MaxY, Data.VertexData.Positions[i].GetY());
					MaxZ = std::max(MaxZ, Data.VertexData.Positions[i].GetZ());

					MinX = std::min(MinX, Data.VertexData.Positions[i].GetX());
					MinY = std::min(MinY, Data.VertexData.Positions[i].GetY());
					MinZ = std::min(MinZ, Data.VertexData.Positions[i].GetZ());
				}
			}
		}

		{
			MaxX = MaxX + MeshAsset->PositiveBoundExtention.GetX();
			MaxY = MaxY + MeshAsset->PositiveBoundExtention.GetY();
			MaxZ = MaxZ + MeshAsset->PositiveBoundExtention.GetZ();

			MinX = MinX - MeshAsset->NegativeBoundExtention.GetX();
			MinY = MinY - MeshAsset->NegativeBoundExtention.GetY();
			MinZ = MinZ - MeshAsset->NegativeBoundExtention.GetZ();

			bool ClippingX = MinX > MaxX;
			bool ClippingY = MinY > MaxY;
			bool ClippingZ = MinZ > MaxZ;

			MaxX = ClippingX ? 1 : MaxX;
			MaxY = ClippingY ? 1 : MaxY;
			MaxZ = ClippingZ ? 1 : MaxZ;

			MinX = ClippingX ? -1 : MinX;
			MinY = ClippingY ? -1 : MinY;
			MinZ = ClippingZ ? -1 : MinZ;

			Vector Center = Vector((MinX + MaxX) / 2, (MinY + MaxY) / 2, (MinZ + MaxZ) / 2);
			Vector Extent = Vector(MaxX, MaxY, MaxZ) - Center;
			float Radius = std::max(std::max(Extent.GetX(), Extent.GetY()), Extent.GetZ());

			Extent = Vector(std::max(Extent.GetX(), MIN_BOX_BOUNDS), std::max(Extent.GetY(), MIN_BOX_BOUNDS), std::max(Extent.GetZ(), MIN_BOX_BOUNDS));
			Radius = std::max(Radius, MIN_SPHERE_BOUNDS);

			MeshAsset->Bounds = BoxSphereBounds(Center, Extent, Radius);
		}

		{
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