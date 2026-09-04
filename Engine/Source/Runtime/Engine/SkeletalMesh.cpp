#include "DrnPCH.h"
#include "SkeletalMesh.h"

#include "Editor/AssetImporter/AssetImporterSkeletalMesh.h"
#include "Editor/AssetPreview/AssetPreviewSkeletalMeshGuiLayer.h"

namespace Drn
{
	SkeletalMesh::SkeletalMesh(const std::string& InPath)
		: Asset(InPath)
		, m_RenderStateDirty(true)
	{
		Load();
	}

#if WITH_EDITOR
	SkeletalMesh::SkeletalMesh( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
	{
		m_SourcePath = InSourcePath;

		Import();
		Save();
	}
#endif

	SkeletalMesh::~SkeletalMesh()
	{
#if WITH_EDITOR
		CloseAssetPreview();
#endif
	}

	void SkeletalMesh::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_SourcePath;
			Ar >> Bounds;

			{
				BufferArchive BuffAr(0);
				Ar >> BuffAr;
				BuffAr.Decompress();
				Data.Serialize(BuffAr);
			}

			{
				BufferArchive BuffAr(0);
				Ar >> BuffAr;
				BuffAr.Decompress();
			}

			{
				Ar >> ImportScale;

				Ar >> m_ImportNormals;
				Ar >> m_ImportTangents;
				Ar >> m_ImportColor;
				Ar >> m_ImportUVs;

				Ar >> PositiveBoundExtention;
				Ar >> NegativeBoundExtention;
			}
		}

		else
		{
			Ar << m_SourcePath;
			Ar << Bounds;

			{
				BufferArchive BufArr(10, false);
				Data.Serialize( BufArr );
				BufArr.Compress();
				Ar << BufArr;
			}

			{
				BufferArchive BufArr(10, false);
				BufArr.Compress();
				Ar << BufArr;
			}

			{
				Ar << ImportScale;

				Ar << m_ImportNormals;
				Ar << m_ImportTangents;
				Ar << m_ImportColor;
				Ar << m_ImportUVs;

				Ar << PositiveBoundExtention;
				Ar << NegativeBoundExtention;
			}
		}
	}

	EAssetType SkeletalMesh::GetAssetType()
	{
		return EAssetType::SkeletalMesh;
	}

	MaterialSlot SkeletalMesh::GetMaterialAtIndex( uint32 Index )
	{
		if (Index >= 0 && Index < Data.Materials.size())
		{
			return Data.Materials[Index];
		}

		return MaterialSlot();
	}

#if WITH_EDITOR
	void SkeletalMesh::Import()
	{
		AssetImporterSkeletalMesh::Import(this, m_SourcePath);
		Save();
		Load();

		MarkRenderStateDirty();
	}

	void SkeletalMesh::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewSkeletalMeshGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void SkeletalMesh::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}
#endif

	//SkeletalMeshSlotData::SkeletalMeshSlotData()
	//{
	//	
	//}
	//
	//SkeletalMeshSlotData::~SkeletalMeshSlotData()
	//{
	//	
	//}

	Archive& operator<<( Archive& Ar, const MeshBoneInfo& Value )
	{
		Ar << Value.Name;
		Ar << Value.ParentIndex;
		return Ar;
	}

	Archive& operator>>(Archive& Ar, MeshBoneInfo& Value)
	{
		Ar >> Value.Name;
		Ar >> Value.ParentIndex;
		return Ar;
	}

	Archive& operator<<(Archive& Ar, const ReferenceSkeleton& Value)
	{
		Ar.operator<< <uint8>(Value.BoneInfo);
		Ar.operator<< <uint8>(Value.BonePose);
		return Ar;
	}

	Archive& operator>>(Archive& Ar, ReferenceSkeleton& Value)
	{
		Ar.operator>> <uint8>(Value.BoneInfo);
		Ar.operator>> <uint8>(Value.BonePose);
		return Ar;
	}

	void SkeletalMeshSlotData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			VertexData.Serialize(Ar);
			Ar >> MaterialIndex;
		}
		else
		{
			VertexData.Serialize(Ar);
			Ar << MaterialIndex;
		}
	}

	void SkeletalMeshData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar.Serialize<uint8>(MeshesData);
			Ar.Serialize<uint8>(Materials);
			Ar >> RefSkeleton;
		}

		else
		{
			Ar.Serialize<uint8>(MeshesData);
			Ar.Serialize<uint8>(Materials);
			Ar << RefSkeleton;
		}
	}

	bool ReferenceSkeleton::HasBone( const std::string& Name ) const
	{
		return FindBone(Name) != -1;
	}

	bool ReferenceSkeleton::IsLeafBone( int32 BoneIndex ) const
	{
		auto It = std::find_if(BoneInfo.begin(), BoneInfo.end(), [&BoneIndex](const MeshBoneInfo& Value){ return BoneIndex == Value.ParentIndex; });
		return It == BoneInfo.end();
	}

	int32 ReferenceSkeleton::FindBone( const std::string& Name ) const
	{
		auto It = std::find(BoneInfo.begin(), BoneInfo.end(), MeshBoneInfo(Name, -1));
		return It == BoneInfo.end() ? -1 : std::distance(BoneInfo.begin(), It);
	}

}  // namespace Drn