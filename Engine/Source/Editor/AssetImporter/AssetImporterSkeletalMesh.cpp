#include "DrnPCH.h"
#include "AssetImporterSkeletalMesh.h"

#if WITH_EDITOR

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

LOG_DEFINE_CATEGORY( LogSkeletalMeshImporter, "SkeletalMeshImporter" );

#define MIN_SPHERE_BOUNDS 0.05f
#define MIN_BOX_BOUNDS 0.01f

namespace Drn
{
	Vector A2Vector(const aiVector3D& InVector) { return Vector(InVector.x, InVector.y, InVector.z); }
	Quat A2Quat(const aiQuaternion& InQuat) { return Quat(InQuat.x, InQuat.y, InQuat.z, InQuat.w); }
	Matrix A2Matrix(const aiMatrix4x4& InMatrix)
	{
		return Matrix(
			Vector4(InMatrix.a1, InMatrix.a2, InMatrix.a3, InMatrix.a4),
			Vector4(InMatrix.b1, InMatrix.b2, InMatrix.b3, InMatrix.b4),
			Vector4(InMatrix.c1, InMatrix.c2, InMatrix.c3, InMatrix.c4),
			Vector4(InMatrix.d1, InMatrix.d2, InMatrix.d3, InMatrix.d4)).GetTranspose();
	}

	void AssetImporterSkeletalMesh::Import( SkeletalMesh* MeshAsset, const std::string& Path )
	{
		ImportedSkeletalMeshData Data;
		
		if (!FileSystem::FileExists(Path))
		{
			LOG(LogSkeletalMeshImporter, Error, "source not found.\n importing failed.");
			return;
		}

		Assimp::Importer importer;
		importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, MeshAsset->ImportScale);
		const aiScene* scene = importer.ReadFile( Path, aiProcess_GlobalScale | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded );

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
		{
			LOG(LogSkeletalMeshImporter, Error, "%s\nimporting failed.", importer.GetErrorString());
			return;
		}

		ProcessSkeleton(MeshAsset, scene, Data);

		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* Mesh = scene->mMeshes[i];

			if ((Mesh->mNumVertices > 0) && Mesh->HasPositions() && Mesh->HasBones())
			{
				ProcessMesh( MeshAsset, scene->mMeshes[i], scene, Data);
			}
		}

		Build(MeshAsset, Data);
	}

	void AssetImporterSkeletalMesh::ProcessSkeleton( SkeletalMesh* MeshAsset, const aiScene* scene, ImportedSkeletalMeshData& BuildingData )
	{
		for (int32 MeshIndex = 0; MeshIndex < scene->mNumMeshes; MeshIndex++)
		{
			aiMesh* Mesh = scene->mMeshes[MeshIndex];
		
			const int32 BoneCount = Mesh->mNumBones;
			for (int32 BoneIndex = 0; BoneIndex < Mesh->mNumBones; BoneIndex++)
			{
				aiBone* Bone = Mesh->mBones[BoneIndex];
		
				if (BuildingData.RefSkeleton.FindBone(Bone->mName.C_Str()) == -1)
				{
					BuildingData.RefSkeleton.BoneInfo.push_back({});
					BuildingData.RefSkeleton.BoneInfo.back().Name = Bone->mName.C_Str();

					BuildingData.RefSkeleton.BonePose.push_back({});
					BuildingData.RefSkeleton.BonePose.back() = A2Matrix(Bone->mOffsetMatrix);
				}
			}
		}

		const int32 BoneCount = BuildingData.RefSkeleton.BoneInfo.size();
		aiNode* Node = scene->mRootNode;
		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			MeshBoneInfo& Bone = BuildingData.RefSkeleton.BoneInfo[BoneIndex];

			aiNode* BoneNode = Node->FindNode(Bone.Name.c_str());
			drn_check(BoneNode);
			aiNode* BoneParentNode = BoneNode->mParent;

			Bone.ParentIndex = BoneParentNode ? BuildingData.RefSkeleton.FindBone(BoneParentNode->mName.C_Str()) : -1;
		}

		ReferenceSkeleton OldSkeleton = BuildingData.RefSkeleton;

		std::vector<std::pair<uint32, uint32>> IndexBoneDepth;
		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			uint32 BoneDepth = 0;
			int32 Parent = OldSkeleton.BoneInfo[BoneIndex].ParentIndex;

			while (Parent != -1)
			{
				BoneDepth++;
				Parent = OldSkeleton.BoneInfo[Parent].ParentIndex;
			}

			IndexBoneDepth.push_back({BoneIndex, BoneDepth});
		}

		std::sort( IndexBoneDepth.begin(), IndexBoneDepth.end(),
			[](const std::pair<uint32, uint32>& A, const std::pair<uint32, uint32>& B) { return A.second < B.second; } );

		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			int32 OldIndex = IndexBoneDepth[BoneIndex].first;
			drn_check(OldIndex >= 0);

			BuildingData.RefSkeleton.BoneInfo[BoneIndex] = OldSkeleton.BoneInfo[OldIndex];
			BuildingData.RefSkeleton.BonePose[BoneIndex] = OldSkeleton.BonePose[OldIndex];

			int32 OldParentIndex = OldSkeleton.BoneInfo[OldIndex].ParentIndex;
			if (OldParentIndex >= 0)
			{
				BuildingData.RefSkeleton.BoneInfo[BoneIndex].ParentIndex = BuildingData.RefSkeleton.FindBone(OldSkeleton.BoneInfo[OldParentIndex].Name);
			}
		}

		//for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		//{
		//	const int32 ParentIndex = BuildingData.RefSkeleton.BoneInfo[BoneIndex].ParentIndex;
		//	if (ParentIndex != -1)
		//	{
		//		BuildingData.RefSkeleton.BonePose[BoneIndex] = BuildingData.RefSkeleton.BonePose[BoneIndex] * BuildingData.RefSkeleton.BonePose[ParentIndex];
		//	}
		//}
		//for (int32 BoneIndex = 0; BoneIndex < BuildingData.RefSkeleton.BoneInfo.size(); BoneIndex++)
		//{
		//	BuildingData.RefSkeleton.BonePose[BoneIndex] = Matrix(BuildingData.RefSkeleton.BonePose[BoneIndex]).Inverse();
		//}
	}

	void AssetImporterSkeletalMesh::ProcessSkeleton( SkeletalMesh* MeshAsset, int32 ParentIndex, const aiNode* node, ImportedSkeletalMeshData& BuildingData )
	{
		auto& Bones = BuildingData.RefSkeleton.BoneInfo;

		int32 BoneIndex = Bones.size();
		Bones.push_back(MeshBoneInfo(node->mName.C_Str(), ParentIndex));
		Transform BoneTransform = A2Matrix(node->mTransformation);
		BoneTransform.SetLocation(BoneTransform.GetLocation());
		BuildingData.RefSkeleton.BonePose.push_back(BoneTransform);

		for (int32 i = 0; i < node->mNumChildren; i++)
		{
			ProcessSkeleton(MeshAsset, BoneIndex, node->mChildren[i], BuildingData);
		}
	}

	void AssetImporterSkeletalMesh::ProcessMesh( SkeletalMesh* MeshAsset, aiMesh* mesh, const aiScene* scene, ImportedSkeletalMeshData& BuildingData )
	{
		std::string Name( mesh->mName.C_Str() );
		uint32 i = Name.find_first_of("_");
		std::string Prefix = Name.substr(0, i);

		BuildingData.MeshesData.push_back({});
		ImportedSkeletalMeshSlotData& MeshData = BuildingData.MeshesData.back();

		const uint32 VertexCount = mesh->mNumVertices;
		MeshData.VertexData.Positions.resize(VertexCount);

		MeshData.VertexData.Normals.resize(MeshAsset->m_ImportNormals ? VertexCount : 0);
		MeshData.VertexData.Tangents.resize(MeshAsset->m_ImportTangents ? VertexCount : 0);
		MeshData.VertexData.Colors.resize(MeshAsset->m_ImportColor? VertexCount : 0);

		MeshData.VertexData.UV_1.resize(MeshAsset->m_ImportUVs >= 1 ? VertexCount : 0);
		MeshData.VertexData.UV_2.resize(MeshAsset->m_ImportUVs >= 2 ? VertexCount : 0);
		MeshData.VertexData.UV_3.resize(MeshAsset->m_ImportUVs >= 3 ? VertexCount : 0);
		MeshData.VertexData.UV_4.resize(MeshAsset->m_ImportUVs >= 4 ? VertexCount : 0);

		MeshData.VertexData.BoneIndices.resize(VertexCount * MAX_EFFECTIVE_BONES);
		MeshData.VertexData.BoneWeights.resize(VertexCount * MAX_EFFECTIVE_BONES);

		for ( uint32 i = 0; i < VertexCount; i++ )
		{
			MeshData.VertexData.Positions[i] = A2Vector(mesh->mVertices[i]);

			if (MeshAsset->m_ImportNormals)
			{
				MeshData.VertexData.Normals[i] = Math::PackSignedNormalizedVectorToUint32(mesh->HasNormals() ? A2Vector(mesh->mNormals[i]) : Vector::UpVector);
			}

			if (MeshAsset->m_ImportTangents)
			{
				MeshData.VertexData.Tangents[i] = Math::PackSignedNormalizedVectorToUint32(mesh->HasTangentsAndBitangents() ? A2Vector(mesh->mTangents[i]) : Vector::UpVector);
			}

			if (MeshAsset->m_ImportColor)
			{
				MeshData.VertexData.Colors[i] = Vector4(mesh->HasVertexColors(0) ? Vector4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a) : Vector4::OneVector);
			}

			std::vector<Vector2Half>* UVs[4] = { &MeshData.VertexData.UV_1, &MeshData.VertexData.UV_2, &MeshData.VertexData.UV_3, &MeshData.VertexData.UV_4 };
			for (int32 UVIndex = 0; UVIndex < 4; UVIndex++)
			{
				if (MeshAsset->m_ImportUVs >= UVIndex + 1)
				{
					(*UVs[UVIndex])[i] = Vector2Half(mesh->HasTextureCoords(UVIndex) ? Vector2Half(mesh->mTextureCoords[UVIndex][i].x, mesh->mTextureCoords[UVIndex][i].y) : Vector2Half::ZeroVector);
				}
			}
		}

		const uint32 BoneCount = mesh->mNumBones;
		drn_check(BoneCount < MAX_BONES);

		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			const int32 ActualBoneIndex = BuildingData.RefSkeleton.FindBone(mesh->mBones[BoneIndex]->mName.C_Str());

			for (int32 BoneVertexIndex = 0; BoneVertexIndex < mesh->mBones[BoneIndex]->mNumWeights; BoneVertexIndex++)
			{
				const uint32 VertexIndex = mesh->mBones[BoneIndex]->mWeights[BoneVertexIndex].mVertexId;
				const float Weight = mesh->mBones[BoneIndex]->mWeights[BoneVertexIndex].mWeight;

				drn_check(Weight > 0.0f);
				drn_check(Weight <= 1.0f);
				drn_check(VertexIndex < VertexCount);

				bool bFound = false;
				for (int32 i = 0; i < MAX_EFFECTIVE_BONES; i++)
				{
					if (!bFound && (MeshData.VertexData.BoneWeights[VertexIndex * MAX_EFFECTIVE_BONES + i].Value == 0))
					{
						MeshData.VertexData.BoneIndices[VertexIndex * MAX_EFFECTIVE_BONES + i] = ActualBoneIndex;
						MeshData.VertexData.BoneWeights[VertexIndex * MAX_EFFECTIVE_BONES + i] = Weight;

						bFound = true;
					}
				}

				drn_check(bFound);
			}
		}

		// adjust normalized weights to account for conversion from float to 8bit data loss
		for ( uint32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++ )
		{
			uint32 SumWeight = 0;
			for (int32 i = 0; i < MAX_EFFECTIVE_BONES; i++)
			{
				SumWeight += MeshData.VertexData.BoneWeights[VertexIndex * MAX_EFFECTIVE_BONES + i].Value;
			}

			float NormalizedError = UINT8_MAX - SumWeight;
			MeshData.VertexData.BoneWeights[VertexIndex * MAX_EFFECTIVE_BONES].Value += NormalizedError;
		}

		MeshData.VertexData.bUse4BitIndices = VertexCount > UINT16_MAX;
		for ( uint32 i = 0; i < mesh->mNumFaces; i++ )
		{
			aiFace face = mesh->mFaces[i];
			for (uint32 j = 0; j < face.mNumIndices; j++)
			{
				if (MeshData.VertexData.bUse4BitIndices)
				{
					MeshData.VertexData.Indices_32.push_back(face.mIndices[j]);
				}
				else
				{
					MeshData.VertexData.Indices_16.push_back(face.mIndices[j]);
				}
			}
		}

		if ( mesh->mMaterialIndex >= 0 )
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::string MatName = material->GetName().C_Str();
			MeshData.MaterialIndex = BuildingData.AddMaterial(MatName);
		}

		MeshData.Bound = Box(A2Vector(mesh->mAABB.mMin), A2Vector(mesh->mAABB.mMax));
	}

	void AssetImporterSkeletalMesh::Build( SkeletalMesh* MeshAsset, ImportedSkeletalMeshData& BuildingData )
	{
		std::vector<MaterialProperty> OldMaterials = MeshAsset->Data.Materials;

		MeshAsset->Data.MeshesData.clear();
		MeshAsset->Data.MeshesData.resize(BuildingData.MeshesData.size());
		for (int32 MeshIndex = 0; MeshIndex < BuildingData.MeshesData.size(); MeshIndex++)
		{
			MeshAsset->Data.MeshesData[MeshIndex].VertexData = BuildingData.MeshesData[MeshIndex].VertexData;
			MeshAsset->Data.MeshesData[MeshIndex].MaterialIndex = BuildingData.MeshesData[MeshIndex].MaterialIndex;
		}

		MeshAsset->Data.Materials.clear();
		MeshAsset->Data.Materials.resize(BuildingData.MaterialsData.size());
		for (int i = 0; i < BuildingData.MaterialsData.size(); i++)
		{
			const std::string& MatName = BuildingData.MaterialsData[i];

			MaterialProperty& M = MeshAsset->Data.Materials[i];
			M.m_Name = MatName;

			std::string MaterialPath = "";

			for (MaterialProperty& OldMaterial : OldMaterials)
			{
				if (OldMaterial.m_Name == MatName)
				{
					M = OldMaterial;
					M.Load();
				}
			}
		}

		MeshAsset->Data.RefSkeleton = BuildingData.RefSkeleton;

		{
			float MaxX = -FLT_MAX, MaxY = -FLT_MAX, MaxZ = -FLT_MAX;
			float MinX = FLT_MAX, MinY = FLT_MAX, MinZ = FLT_MAX;
		
			for (auto& Mesh : BuildingData.MeshesData)
			{
				MaxX = std::max(MaxX, Mesh.Bound.Max.X);
				MaxY = std::max(MaxY, Mesh.Bound.Max.Y);
				MaxZ = std::max(MaxZ, Mesh.Bound.Max.Z);
		
				MinX = std::min(MinX, Mesh.Bound.Min.X);
				MinY = std::min(MinY, Mesh.Bound.Min.Y);
				MinZ = std::min(MinZ, Mesh.Bound.Min.Z);
			}
		
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
			Extent = Vector(std::max(Extent.GetX(), MIN_BOX_BOUNDS), std::max(Extent.GetY(), MIN_BOX_BOUNDS), std::max(Extent.GetZ(), MIN_BOX_BOUNDS));
		
			float Radius = Extent.Length();
			Radius = std::max(Radius, MIN_SPHERE_BOUNDS);
		
			MeshAsset->Bounds = BoxSphereBounds(Center, Extent, Radius);
		}
	}

	uint8 ImportedSkeletalMeshData::AddMaterial( std::string& InMaterial )
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

// ----------------------------------------------------------------------------------------------------------------

	void AssetImporterSkeletalMesh::ImportAnimation( AnimationSequence* AnimationAsset, SkeletalMesh* MeshAsset, const std::string& Path )
	{
		if (!FileSystem::FileExists(Path))
		{
			LOG(LogSkeletalMeshImporter, Error, "source not found.\n importing failed.");
			return;
		}

		Assimp::Importer importer;
		importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, MeshAsset->ImportScale);
		const aiScene* scene = importer.ReadFile( Path, aiProcess_GlobalScale | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded );

		//if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
		if(!scene || !scene->mRootNode) 
		{
			LOG(LogSkeletalMeshImporter, Error, "%s\nimporting failed.", importer.GetErrorString());
			return;
		}

		drn_check(scene->mNumAnimations == 1);
		aiAnimation* Animation = scene->mAnimations[0];

		AnimationData& AnimData = AnimationAsset->Data;
		ReferenceSkeleton& RefSkeleton = MeshAsset->Data.RefSkeleton;

		const float TicksPerSecond = Animation->mTicksPerSecond;
		AnimData.Length = Animation->mDuration / TicksPerSecond;
		const int32 NumFrames = TicksPerSecond * AnimData.Length + 1;

		const int32 BoneCount = RefSkeleton.BoneInfo.size();
		AnimData.KeyFrames.resize(NumFrames);
		for (int32 FrameNumer = 0; FrameNumer < NumFrames; FrameNumer++)
		{
			AnimData.KeyFrames[FrameNumer].BonePose.resize(RefSkeleton.BoneInfo.size());
		}

		// fill with ref skeleton data
		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			Transform BoneDefaultTransform = RefSkeleton.GetParentBoneSpaceTransform(BoneIndex);
			for (int32 FrameNumer = 0; FrameNumer < NumFrames; FrameNumer++)
			{
				AnimData.KeyFrames[FrameNumer].BonePose[BoneIndex] = RefSkeleton.GetParentBoneSpaceTransform(BoneIndex);
			}
		}

		struct FilledData
		{
			bool bLocation = false;
			bool bRotation = false;
			bool bScale = false;
		};

		std::vector<std::vector<FilledData>> FramesFilledData;
		FramesFilledData.resize(NumFrames);
		for (int32 FrameIndex = 0; FrameIndex < NumFrames; FrameIndex++)
		{
			FramesFilledData[FrameIndex].resize(BoneCount);
		}

		int32 KeyIndex1; int32 KeyIndex2; float Alpha;
		for (int32 ChannelIndex = 0; ChannelIndex < Animation->mNumChannels; ChannelIndex++)
		{
			aiNodeAnim* Channel = Animation->mChannels[ChannelIndex];
			std::string BoneName = Channel->mNodeName.C_Str();
			const int32 BoneIndex = RefSkeleton.FindBone(BoneName);
			if (BoneIndex == -1)
			{
				//drn_check(false);
				continue;
			}

			for (int32 PositionKeyIndex = 0; PositionKeyIndex < Channel->mNumPositionKeys; PositionKeyIndex++)
			{
				AnimationRuntime::GetFrameIndicesFromTime(KeyIndex1, KeyIndex2, Alpha, Channel->mPositionKeys[PositionKeyIndex].mTime / TicksPerSecond, NumFrames, AnimData.Length);

				const int32 NumKeys = NumFrames - 1;
				const float KeyPos = ((float)NumKeys * Channel->mPositionKeys[PositionKeyIndex].mTime / TicksPerSecond) / AnimData.Length;
				const int32 KeyIndex1 = std::clamp<int32>( std::round(KeyPos), 0, NumFrames-1 );

				AnimData.KeyFrames[KeyIndex1].BonePose[BoneIndex].SetLocation(A2Vector(Channel->mPositionKeys[PositionKeyIndex].mValue));

				FramesFilledData[KeyIndex1][BoneIndex].bLocation = true;
			}

			for (int32 RotationKeyIndex = 0; RotationKeyIndex < Channel->mNumRotationKeys; RotationKeyIndex++)
			{
				AnimationRuntime::GetFrameIndicesFromTime(KeyIndex1, KeyIndex2, Alpha, Channel->mRotationKeys[RotationKeyIndex].mTime / TicksPerSecond, NumFrames, AnimData.Length);

				const int32 NumKeys = NumFrames - 1;
				const float KeyPos = ((float)NumKeys * Channel->mPositionKeys[RotationKeyIndex].mTime / TicksPerSecond) / AnimData.Length;
				const int32 KeyIndex1 = std::clamp<int32>( std::round(KeyPos), 0, NumFrames-1 );

				AnimData.KeyFrames[KeyIndex1].BonePose[BoneIndex].SetRotation(A2Quat(Channel->mRotationKeys[RotationKeyIndex].mValue));

				FramesFilledData[KeyIndex1][BoneIndex].bRotation = true;
			}

			for (int32 ScaleKeyIndex = 0; ScaleKeyIndex < Channel->mNumScalingKeys; ScaleKeyIndex++)
			{
				AnimationRuntime::GetFrameIndicesFromTime(KeyIndex1, KeyIndex2, Alpha, Channel->mScalingKeys[ScaleKeyIndex].mTime / TicksPerSecond, NumFrames, AnimData.Length);

				const int32 NumKeys = NumFrames - 1;
				const float KeyPos = ((float)NumKeys * Channel->mPositionKeys[ScaleKeyIndex].mTime / TicksPerSecond) / AnimData.Length;
				const int32 KeyIndex1 = std::clamp<int32>( std::round(KeyPos), 0, NumFrames-1 );

				AnimData.KeyFrames[KeyIndex1].BonePose[BoneIndex].SetScale(A2Vector(Channel->mScalingKeys[ScaleKeyIndex].mValue));

				FramesFilledData[KeyIndex1][BoneIndex].bScale = true;
			}
		}

		// fill empty frame data
		for (int32 BoneIndex = 0; BoneIndex < BoneCount; BoneIndex++)
		{
			const Vector BoneDefaultLocation	= AnimData.KeyFrames[0].BonePose[BoneIndex].GetLocation();
			const Quat BoneDefaultRotation		= AnimData.KeyFrames[0].BonePose[BoneIndex].GetRotation();
			const Vector BoneDefaultScale		= AnimData.KeyFrames[0].BonePose[BoneIndex].GetScale();

			for (int32 FrameIndex = 1; FrameIndex < NumFrames; FrameIndex++)
			{
				if (!FramesFilledData[FrameIndex][BoneIndex].bLocation)
				{
					AnimData.KeyFrames[FrameIndex].BonePose[BoneIndex].SetLocation(BoneDefaultLocation);
				}

				if (!FramesFilledData[FrameIndex][BoneIndex].bRotation)
				{
					AnimData.KeyFrames[FrameIndex].BonePose[BoneIndex].SetRotation(BoneDefaultRotation);
				}

				if (!FramesFilledData[FrameIndex][BoneIndex].bScale)
				{
					AnimData.KeyFrames[FrameIndex].BonePose[BoneIndex].SetScale(BoneDefaultScale);
				}
			}
		}
	}

        }

#endif