#pragma once

#include "ForwardTypes.h"

#if WITH_EDITOR

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

LOG_DECLARE_CATEGORY( LogMeshImporterHelper );

namespace Drn
{
	enum class EMeshImporterPreviewFlags : uint32
	{
		None				= 1 << 0,
		HasStaticMeshes		= 1 << 0,
		HasSkeletalMeshes	= 1 << 1,
		IsValidSource		= 1 << 2,
	};

	class MeshImporterHelper
	{
	public:
		static EMeshImporterPreviewFlags PreviewMeshSource(const std::string& SourcePath);
	};

	inline Vector A2Vector(const aiVector3D& Vec) { return Vector(Vec.x, Vec.y, Vec.z); }
	inline Quat A2Quat(const aiQuaternion& InQuat) { return Quat(InQuat.x, InQuat.y, InQuat.z, InQuat.w); }
	inline Matrix A2Matrix(const aiMatrix4x4& InMatrix)
	{
		return Matrix(
			Vector4(InMatrix.a1, InMatrix.a2, InMatrix.a3, InMatrix.a4),
			Vector4(InMatrix.b1, InMatrix.b2, InMatrix.b3, InMatrix.b4),
			Vector4(InMatrix.c1, InMatrix.c2, InMatrix.c3, InMatrix.c4),
			Vector4(InMatrix.d1, InMatrix.d2, InMatrix.d3, InMatrix.d4)).GetTranspose();
	}

}

#endif