#include "DrnPCH.h"
#include "AssetImporterMaterial.h"

#if WITH_EDITOR

LOG_DEFINE_CATEGORY( LogAssetImporterMaterial, "AssetImporterMaterial" );

namespace Drn
{
	void AssetImporterMaterial::Import( Material* MaterialAsset, const std::string& Path )
	{
		std::string ShaderString = FileSystem::ReadFileAsString(Path);

		bool HasVS = ShaderString.find("Main_VS") != std::string::npos;
		bool HasPS = ShaderString.find("Main_PS") != std::string::npos;
		bool HasGS = ShaderString.find("Main_GS") != std::string::npos;
		bool HasHS = ShaderString.find("Main_HS") != std::string::npos;
		bool HasDS = ShaderString.find("Main_DS") != std::string::npos;
		bool HasCS = ShaderString.find("Main_CS") != std::string::npos;

		bool Successed = true;

		ID3DBlob* VS_Blob = nullptr;
		ID3DBlob* PS_Blob = nullptr;
		ID3DBlob* GS_Blob = nullptr;
		ID3DBlob* HS_Blob = nullptr;
		ID3DBlob* DS_Blob = nullptr;

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

		if (Successed)
		{
			MaterialAsset->ReleaseShaderBlobs();

			MaterialAsset->m_ShaderBlob.m_VS = VS_Blob;
			MaterialAsset->m_ShaderBlob.m_PS = PS_Blob;
			MaterialAsset->m_ShaderBlob.m_GS = GS_Blob;
			MaterialAsset->m_ShaderBlob.m_HS = HS_Blob;
			MaterialAsset->m_ShaderBlob.m_DS = DS_Blob;

			UpdateMaterialParameterSlots(MaterialAsset, ShaderString);
		}

		else
		{
			if (VS_Blob) VS_Blob->Release();
			if (PS_Blob) PS_Blob->Release();
			if (GS_Blob) GS_Blob->Release();
			if (HS_Blob) HS_Blob->Release();
			if (DS_Blob) DS_Blob->Release();
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

	void AssetImporterMaterial::UpdateMaterialParameterSlots( Material* MaterialAsset, const std::string& ShaderCode )
	{
		std::vector<Texture2DProperty> OldTexture2Ds = MaterialAsset->m_Texture2DSlots;
		MaterialAsset->m_Texture2DSlots.clear();

		std::vector<std::string> NamedTokens;
		FindNamedTokens(ShaderCode, "Texture2D", NamedTokens);

		for (const std::string& name : NamedTokens)
		{
			std::string Path = DEFAULT_TEXTURE_PATH;

			for (const Texture2DProperty& OldTexture2D : OldTexture2Ds)
			{
				if (OldTexture2D.m_Name == name)
				{
					Path = OldTexture2D.m_Texture2D.GetPath();
				}
			}

			MaterialAsset->m_Texture2DSlots.push_back(Texture2DProperty(name, Path));
		}

// ----------------------------------------------------------------------------------------------------------------------------

		std::vector<MaterialIndexedFloatParameter> OldScalars = MaterialAsset->m_FloatSlots;
		MaterialAsset->m_FloatSlots.clear();

		NamedTokens.clear();
		FindNamedTokens(ShaderCode, "//Scalar", NamedTokens);

		for (int i = 0; i < NamedTokens.size(); i++)
		{
			const std::string& name = NamedTokens[i];
			float Value = 0.0f;

			for (const FloatProperty& OldScalar : OldScalars)
			{
				if (OldScalar.m_Name == name)
				{
					Value = OldScalar.m_Value;
				}
			}

			MaterialAsset->m_FloatSlots.push_back(MaterialIndexedFloatParameter(name, Value, i));
		}

// ----------------------------------------------------------------------------------------------------------------------------

		const uint32 FloatSize = MaterialAsset->m_FloatSlots.size();

		std::vector<MaterialIndexedVector4Parameter> OldVector4s = MaterialAsset->m_Vector4Slots;
		MaterialAsset->m_Vector4Slots.clear();

		NamedTokens.clear();
		FindNamedTokens(ShaderCode, "//Vector4", NamedTokens);

		for (int i = 0; i < NamedTokens.size(); i++)
		{
			const std::string& name = NamedTokens[i];
			Vector4 Value = 0.0f;

			for (const Vector4Property& OldVector4 : OldVector4s)
			{
				if (OldVector4.m_Name == name)
				{
					Value = OldVector4.m_Value;
				}
			}

			MaterialAsset->m_Vector4Slots.push_back(MaterialIndexedVector4Parameter(name, Value, FloatSize + i * 4));
		}


	}

	void AssetImporterMaterial::FindNamedTokens( const std::string& ShaderCode, const std::string& Token, std::vector<std::string>& Result )
	{
		const size_t TokenLength = Token.length();

		size_t pos = ShaderCode.find(Token, 0);
		while( pos != std::string::npos )
		{
			size_t NameStart = pos + TokenLength + 1;
			size_t NameEnd = ShaderCode.find_first_of( " ;\n", NameStart);

			if (NameEnd != std::string::npos)
			{
				std::string name = ShaderCode.substr(NameStart, NameEnd - NameStart);
				Result.push_back(name);

				pos = ShaderCode.find(Token, NameEnd);
			}

			else
			{
				pos = std::string::npos;
			}
		}
	}

}

#endif