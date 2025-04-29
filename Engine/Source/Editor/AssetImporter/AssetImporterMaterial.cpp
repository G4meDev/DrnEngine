#include "DrnPCH.h"
#include "AssetImporterMaterial.h"

#if WITH_EDITOR

LOG_DEFINE_CATEGORY( LogAssetImporterMaterial, "AssetImporterMaterial" );

namespace Drn
{
	void AssetImporterMaterial::Import( Material* MaterialAsset, const std::string& Path )
	{
		std::string ShaderString = FileSystem::ReadFileAsString(Path);
		//TODO: identify entry points. ex Main_VS, Main_PS, ...

		bool HasVS = true;
		bool HasPS = true;
		bool HasGS = true;
		bool HasHS = false;
		bool HasDS = false;
		bool HasCS = false;

		bool Successed = true;

		ID3DBlob* VS_Blob = nullptr;
		ID3DBlob* PS_Blob = nullptr;
		ID3DBlob* GS_Blob = nullptr;
		ID3DBlob* HS_Blob = nullptr;
		ID3DBlob* DS_Blob = nullptr;
		ID3DBlob* CS_Blob = nullptr;

		auto CompileShaderBlobConditional = [&](bool Condition, const std::string& InPath, char* InEntryPoint, char* InProfile, ID3DBlob** InByteBlob) 
		{
			if (Condition)
			{
				Successed &= CompileShader( StringHelper::s2ws(InPath), InEntryPoint, InProfile, InByteBlob);
			}
		};

		//TODO: increase shader models
		CompileShaderBlobConditional(HasVS, Path, "Main_VS", "vs_5_1", &VS_Blob);
		CompileShaderBlobConditional(HasPS, Path, "Main_PS", "ps_5_1", &PS_Blob);
		CompileShaderBlobConditional(HasGS, Path, "Main_GS", "gs_5_1", &GS_Blob);
		CompileShaderBlobConditional(HasHS, Path, "Main_HS", "hs_5_1", &HS_Blob);
		CompileShaderBlobConditional(HasDS, Path, "Main_DS", "ds_5_1", &DS_Blob);
		CompileShaderBlobConditional(HasCS, Path, "Main_CS", "cs_5_1", &CS_Blob);

		if (Successed)
		{
			MaterialAsset->ReleaseShaderBlobs();

			MaterialAsset->m_VS_Blob = VS_Blob;
			MaterialAsset->m_PS_Blob = PS_Blob;
			MaterialAsset->m_GS_Blob = GS_Blob;
			MaterialAsset->m_HS_Blob = HS_Blob;
			MaterialAsset->m_DS_Blob = DS_Blob;
			MaterialAsset->m_CS_Blob = CS_Blob;
		}

		else
		{
			if (VS_Blob) VS_Blob->Release();
			if (PS_Blob) PS_Blob->Release();
			if (GS_Blob) GS_Blob->Release();
			if (HS_Blob) HS_Blob->Release();
			if (DS_Blob) DS_Blob->Release();
			if (CS_Blob) CS_Blob->Release();
		}
	}

	bool AssetImporterMaterial::CompileShader( const std::wstring& ShaderPath, char* EntryPoint, char* Profile, ID3DBlob** ByteBlob )
	{
		*ByteBlob = nullptr;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		const D3D_SHADER_MACRO defines[] = 
		{
			"EXAMPLE_DEFINE", "1",
			NULL, NULL
		};

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile( ShaderPath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
			EntryPoint, Profile, flags, 0, &shaderBlob, &errorBlob );

		if ( FAILED(hr) )
		{
			if ( errorBlob )
			{
				LOG(LogAssetImporterMaterial, Error, "Shader compile failed. %s", (char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			if ( shaderBlob )
				shaderBlob->Release();

			return false;
		}

		*ByteBlob = shaderBlob;

		return true;
	}

}

#endif