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

		static bool CompileShader( const std::wstring& ShaderPath, const char* EntryPoint, const char* Profile, const D3D_SHADER_MACRO* Macros, ID3DBlob** ByteBlob );
		static void UpdateMaterialParameterSlots(Material* MaterialAsset, const std::string& ShaderCode);
		static void FindNamedTokens(const std::string& ShaderCode, const std::string& Token, std::vector<std::string>& Result);

	private:
	};
}

#endif