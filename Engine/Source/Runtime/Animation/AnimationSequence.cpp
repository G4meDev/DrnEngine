#include "DrnPCH.h"
#include "AnimationSequence.h"

#include "Editor/AssetImporter/AssetImporterSkeletalMesh.h"
#include "Editor/AssetPreview/AssetPreviewAnimationSequenceGuiLayer.h"

namespace Drn
{
	AnimationSequence::AnimationSequence( const std::string& Path )
		: Asset( Path )
	{
		Load();
	}

#if WITH_EDITOR
	AnimationSequence::AnimationSequence( const std::string& InPath, const std::string& InSourcePath, AssetImportUserData& UserData)
		: Asset( InPath )
	{
		m_SourcePath = InSourcePath;

		std::string SkeletonPath;
		UserData.UserData >> SkeletonPath;

		OwningSkeleton = AssetHandle<SkeletalMesh>(SkeletonPath);
		OwningSkeleton.Load();

		Import();
		Save();
	}
#endif

	AnimationSequence::~AnimationSequence()
	{
#if WITH_EDITOR
		CloseAssetPreview();
#endif
	}

	void AnimationSequence::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_SourcePath;

			std::string SkeletonPath;
			Ar >> SkeletonPath;

			OwningSkeleton = AssetHandle<SkeletalMesh>(SkeletonPath);
			OwningSkeleton.Load();

			//Ar >> Data;

			BufferArchive BuffAr(0);
			Ar >> BuffAr;
			BuffAr.Decompress();
			BuffAr >> Data;
		}
		else
		{
			Ar << m_SourcePath;
			Ar << OwningSkeleton.GetPath();

			//Ar << Data;

			BufferArchive BufArr(10, false);
			BufArr << Data;
			BufArr.Compress();
			Ar << BufArr;
		}
	}

	EAssetType AnimationSequence::GetAssetType()
	{
		return EAssetType::AnimationSequence;
	}

#if WITH_EDITOR
	void AnimationSequence::Import()
	{
		AssetImporterSkeletalMesh::ImportAnimation(this, OwningSkeleton.Get(), m_SourcePath);
		Save();
		Load();

		if (GuiLayer)
		{
			GuiLayer->OnReimport();
		}
	}

	void AnimationSequence::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewAnimationSequenceGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void AnimationSequence::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}

#endif

// -----------------------------------------------------------------------------------------------------

	class Archive& operator<<(Archive& Ar, const AnimationKeyFrame& Value)
	{
		Ar.operator<< <uint8>(Value.BonePose);

		return Ar;
	}

	class Archive& operator>>(Archive& Ar, AnimationKeyFrame& Value)
	{
		Ar.operator>> <uint8>(Value.BonePose);

		return Ar;
	}

	class Archive& operator<<(Archive& Ar, const AnimationData& Value)
	{
		Ar << Value.Length;
		Ar.operator<< <uint32>(Value.KeyFrames);

		return Ar;
	}

	class Archive& operator>>(Archive& Ar, AnimationData& Value)
	{
		Ar >> Value.Length;
		Ar.operator>> <uint32>(Value.KeyFrames);

		return Ar;
	}

        }  // namespace Drn