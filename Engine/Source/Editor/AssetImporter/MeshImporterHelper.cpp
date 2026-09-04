#include "DrnPCH.h"
#include "MeshImporterHelper.h"

#if WITH_EDITOR

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

LOG_DEFINE_CATEGORY( LogMeshImporterHelper, "MeshImporterHelper" );

namespace Drn
{
	EMeshImporterPreviewFlags MeshImporterHelper::PreviewMeshSource( const std::string& SourcePath )
	{
		EMeshImporterPreviewFlags Result = EMeshImporterPreviewFlags::None;

		if (!FileSystem::FileExists(SourcePath))
		{
			LOG(LogMeshImporterHelper, Error, "source not found.\n importing failed.");
			return Result;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile( SourcePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded );

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
		{
			LOG(LogMeshImporterHelper, Error, "%s\nimporting failed.", importer.GetErrorString());
			return Result;
		}

		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			if (scene->mMeshes[i]->HasBones())
			{
				EnumAddFlags(Result, EMeshImporterPreviewFlags::HasSkeletalMeshes);
			}
			else
			{
				EnumAddFlags(Result, EMeshImporterPreviewFlags::HasStaticMeshes);
			}
		}

		if (EnumHasAnyFlags(Result, EMeshImporterPreviewFlags::HasStaticMeshes) || EnumHasAnyFlags(Result, EMeshImporterPreviewFlags::HasSkeletalMeshes))
		{
			EnumAddFlags(Result, EMeshImporterPreviewFlags::IsValidSource);
		}

		return Result;
	}

}

#endif