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
		static void Import(MaterialInstance* MaterialAsset, const std::string& Path);

	protected:

		static bool CompileShader( const std::wstring& ShaderPath, const wchar_t* EntryPoint, const wchar_t* Profile, const std::vector<const wchar_t*>& Macros, ID3DBlob** ByteBlob );
		static void UpdateMaterialParameterSlots(Material* MaterialAsset, const std::string& ShaderCode);

	private:
	};
}

#endif