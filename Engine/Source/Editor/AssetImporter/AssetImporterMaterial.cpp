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

		bool SupportMainPass = ShaderString.find( "SUPPORT_MAIN_PASS" ) != std::string::npos;
		bool SupportHitProxyPass = ShaderString.find( "SUPPORT_HIT_PROXY_PASS" ) != std::string::npos;
		bool SupportEditorPrimitivePass = ShaderString.find( "SUPPORT_EDITOR_PRIMITIVE_PASS" ) != std::string::npos;
		bool SupportEditorSelectionPass = ShaderString.find( "SUPPORT_EDITOR_SELECTION_PASS" ) != std::string::npos;

		bool Successed = true;
		ShaderBlob MainShaderBlob;
		ShaderBlob HitProxyShaderBlob;
		ShaderBlob EditorPrimitiveShaderBlob;

		auto CompileShaderBlobConditional = [&](bool Condition, const std::string& InPath,
			char* InEntryPoint, char* InProfile, const D3D_SHADER_MACRO* Macros, ID3DBlob** InByteBlob) 
		{
			if (Condition)
			{
				Successed &= CompileShader( StringHelper::s2ws(InPath), InEntryPoint, InProfile, Macros, InByteBlob);
			}
		};

		D3D_SHADER_MACRO MainMacros[] = { "MAIN_PASS", "1", NULL, NULL };

		if (SupportMainPass)
		{
			CompileShaderBlobConditional(HasVS, Path, "Main_VS", "vs_5_1", MainMacros, &MainShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, "Main_PS", "ps_5_1", MainMacros, &MainShaderBlob.m_PS);
			CompileShaderBlobConditional(HasGS, Path, "Main_GS", "gs_5_1", MainMacros, &MainShaderBlob.m_GS);
			CompileShaderBlobConditional(HasHS, Path, "Main_HS", "hs_5_1", MainMacros, &MainShaderBlob.m_HS);
			CompileShaderBlobConditional(HasDS, Path, "Main_DS", "ds_5_1", MainMacros, &MainShaderBlob.m_DS);
		}

		if (SupportHitProxyPass)
		{
			D3D_SHADER_MACRO HitProxyMacros[] = { "HitProxyPass", "1", NULL, NULL };
			CompileShaderBlobConditional(HasVS, Path, "Main_VS", "vs_5_1", HitProxyMacros, &HitProxyShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, "Main_PS", "ps_5_1", HitProxyMacros, &HitProxyShaderBlob.m_PS);
			CompileShaderBlobConditional(HasGS, Path, "Main_GS", "gs_5_1", HitProxyMacros, &HitProxyShaderBlob.m_GS);
			CompileShaderBlobConditional(HasHS, Path, "Main_HS", "hs_5_1", HitProxyMacros, &HitProxyShaderBlob.m_HS);
			CompileShaderBlobConditional(HasDS, Path, "Main_DS", "ds_5_1", HitProxyMacros, &HitProxyShaderBlob.m_DS);
		}

		if (SupportEditorPrimitivePass)
		{
			D3D_SHADER_MACRO HitProxyMacros[] = { "EDITOR_PRIMITIVE_PASS", "1", NULL, NULL };
			CompileShaderBlobConditional(HasVS, Path, "Main_VS", "vs_5_1", HitProxyMacros, &EditorPrimitiveShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, "Main_PS", "ps_5_1", HitProxyMacros, &EditorPrimitiveShaderBlob.m_PS);
			CompileShaderBlobConditional(HasGS, Path, "Main_GS", "gs_5_1", HitProxyMacros, &EditorPrimitiveShaderBlob.m_GS);
			CompileShaderBlobConditional(HasHS, Path, "Main_HS", "hs_5_1", HitProxyMacros, &EditorPrimitiveShaderBlob.m_HS);
			CompileShaderBlobConditional(HasDS, Path, "Main_DS", "ds_5_1", HitProxyMacros, &EditorPrimitiveShaderBlob.m_DS);
		}

		if (Successed)
		{
			MaterialAsset->ReleaseShaderBlobs();
			MaterialAsset->m_MainShaderBlob = MainShaderBlob;
			MaterialAsset->m_HitProxyShaderBlob = HitProxyShaderBlob;
			MaterialAsset->m_EditorPrimitiveShaderBlob = EditorPrimitiveShaderBlob;

			MaterialAsset->m_SupportMainPass = SupportMainPass;
			MaterialAsset->m_SupportHitProxyPass = SupportHitProxyPass;
			MaterialAsset->m_SupportEditorPrimitivePass = SupportEditorPrimitivePass;
			MaterialAsset->m_SupportEditorSelectionPass = SupportEditorSelectionPass;

			UpdateMaterialParameterSlots(MaterialAsset, ShaderString);
		}
	}

	bool AssetImporterMaterial::CompileShader( const std::wstring& ShaderPath, char* EntryPoint, char* Profile, const D3D_SHADER_MACRO* Macros, ID3DBlob** ByteBlob )
	{
		*ByteBlob = nullptr;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile( ShaderPath.c_str(), Macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
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