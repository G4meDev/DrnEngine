#pragma once

#if WITH_EDITOR

LOG_DECLARE_CATEGORY(LogAssetImporterMaterial);

#include "ForwardTypes.h"

namespace Drn
{
	class AssetImporterMaterial
	{
	public:
		static void Import(Material* MaterialAsset, const std::string& Path);

	protected:

		static bool CompileShader( const std::wstring& ShaderPath, char* EntryPoint, char* Profile, ID3DBlob** ByteBlob );

	private:
	};
}

#endif