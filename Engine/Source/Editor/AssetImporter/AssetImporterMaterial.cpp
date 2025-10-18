#include "DrnPCH.h"
#include "AssetImporterMaterial.h"

#if WITH_EDITOR

#include <dxcapi.h>

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
		SupportMainPass |= MaterialAsset->m_MaterialDomain == EMaterialDomain::Decal; // if domain is decal then main pass is used for decal
		bool SupportHitProxyPass = ShaderString.find( "SUPPORT_HIT_PROXY_PASS" ) != std::string::npos;
		bool SupportEditorPrimitivePass = ShaderString.find( "SUPPORT_EDITOR_PRIMITIVE_PASS" ) != std::string::npos;
		bool SupportEditorSelectionPass = ShaderString.find( "SUPPORT_EDITOR_SELECTION_PASS" ) != std::string::npos;
		bool SupportShadowPass = ShaderString.find( "SUPPORT_SHADOW_PASS" ) != std::string::npos;

		bool Successed = true;
		ShaderBlob MainShaderBlob;
		ShaderBlob HitProxyShaderBlob;
		ShaderBlob EditorPrimitiveShaderBlob;
		ShaderBlob PointLightShadowDepthShaderBlob;
		ShaderBlob SpotLightShadowDepthShaderBlob;

		auto CompileShaderBlobConditional = [&](bool Condition, const std::string& InPath,
			const wchar_t* InEntryPoint, const wchar_t* InProfile, const std::vector<const wchar_t*>& Macros, ID3DBlob** InByteBlob) 
		{
			if (Condition)
			{
				Successed &= CompileShader( StringHelper::s2ws(InPath), InEntryPoint, InProfile, Macros, InByteBlob);
			}
		};

		const std::vector<const wchar_t*> MainMacros = { L"MAIN_PASS=1" };

		if (SupportMainPass)
		{
			CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", MainMacros, &MainShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", MainMacros, &MainShaderBlob.m_PS);
			CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", MainMacros, &MainShaderBlob.m_GS);
			CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", MainMacros, &MainShaderBlob.m_HS);
			CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", MainMacros, &MainShaderBlob.m_DS);
		}

		if (SupportHitProxyPass)
		{
			const std::vector<const wchar_t*> HitProxyMacros = { L"HITPROXY_PASS=1" };

			CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", HitProxyMacros, &HitProxyShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", HitProxyMacros, &HitProxyShaderBlob.m_PS);
			CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", HitProxyMacros, &HitProxyShaderBlob.m_GS);
			CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", HitProxyMacros, &HitProxyShaderBlob.m_HS);
			CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", HitProxyMacros, &HitProxyShaderBlob.m_DS);
		}

		if (SupportEditorPrimitivePass)
		{
			const std::vector<const wchar_t*> EditorPrimitiveMacros = { L"EDITOR_PRIMITIVE_PASS=1" };

			CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", EditorPrimitiveMacros, &EditorPrimitiveShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", EditorPrimitiveMacros, &EditorPrimitiveShaderBlob.m_PS);
			CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", EditorPrimitiveMacros, &EditorPrimitiveShaderBlob.m_GS);
			CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", EditorPrimitiveMacros, &EditorPrimitiveShaderBlob.m_HS);
			CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", EditorPrimitiveMacros, &EditorPrimitiveShaderBlob.m_DS);
		}

		if (SupportShadowPass)
		{
			{
				const std::vector<const wchar_t*> PointLightShadowMacros = { L"SHADOW_PASS=1", L"SHADOW_PASS_POINTLIGHT=1" };
				CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_VS);
				CompileShaderBlobConditional(true, Path, L"PointLightShadow_GS", L"gs_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_GS);
				CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_HS);
				CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_DS);
			}

			{
				const std::vector<const wchar_t*> SpotlLightShadowMacros = { L"SHADOW_PASS=1", L"SHADOW_PASS_SPOTLIGHT=1" };
				CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_VS);
				//CompileShaderBlobConditional(true, Path, L"PointLightShadow_GS", L"gs_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_GS);
				CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_HS);
				CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_DS);
			}
		}

		if (Successed)
		{
			MaterialAsset->ReleaseShaderBlobs();
			MaterialAsset->m_MainShaderBlob = MainShaderBlob;
			MaterialAsset->m_HitProxyShaderBlob = HitProxyShaderBlob;
			MaterialAsset->m_EditorPrimitiveShaderBlob = EditorPrimitiveShaderBlob;
			MaterialAsset->m_PointlightShadowDepthShaderBlob = PointLightShadowDepthShaderBlob;
			MaterialAsset->m_SpotlightShadowDepthShaderBlob = SpotLightShadowDepthShaderBlob;

			MaterialAsset->m_SupportMainPass = SupportMainPass;
			MaterialAsset->m_SupportHitProxyPass = SupportHitProxyPass;
			MaterialAsset->m_SupportEditorPrimitivePass = SupportEditorPrimitivePass;
			MaterialAsset->m_SupportEditorSelectionPass = SupportEditorSelectionPass;
			MaterialAsset->m_SupportShadowPass = SupportShadowPass;

			UpdateMaterialParameterSlots(MaterialAsset, ShaderString);
		}
	}

	bool AssetImporterMaterial::CompileShader( const std::wstring& ShaderPath, const wchar_t* EntryPoint, const wchar_t* Profile, const std::vector<const wchar_t*>& Macros, ID3DBlob** ByteBlob )
	{
		Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
		Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;

		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddressOf()));

		Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
		pUtils->CreateDefaultIncludeHandler(pIncludeHandler.GetAddressOf());

		std::vector<LPCWSTR> Args;
		Args.push_back(L"Name");

		Args.push_back(L"-E");
		Args.push_back(EntryPoint);

		Args.push_back(L"-T");
		Args.push_back(Profile);

		Args.push_back(L"-Zs");

		Args.push_back(L"-O3");

		Args.push_back(L"-I ..\\..\\..\\Engine\\Content\\Materials\\");

		for (auto& Mac : Macros)
		{
			Args.push_back(L"-D");
			Args.push_back(Mac);
		}

		//Args.push_back(L"-Fo");
		//Args.push_back( L"myshader.bin");
		//
		//Args.push_back(L"-Fd");
		//Args.push_back(L"myshader.pdb");

		Args.push_back(L"-Qstrip_reflect");

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource = nullptr;
		pUtils->LoadFile(ShaderPath.c_str(), nullptr, &pSource);
		DxcBuffer Source;
		Source.Ptr = pSource->GetBufferPointer();
		Source.Size = pSource->GetBufferSize();
		Source.Encoding = DXC_CP_ACP;

		Microsoft::WRL::ComPtr<IDxcResult> pResults;
		pCompiler->Compile(
			&Source,
			Args.data(),
			Args.size(),
			pIncludeHandler.Get(),
			IID_PPV_ARGS(pResults.GetAddressOf())
		);

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
		pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		if (pErrors != nullptr && pErrors->GetStringLength() != 0)
		{
			LOG(LogAssetImporterMaterial, Warning, "\"%ws\" complation log:\n%s", ShaderPath.c_str(), pErrors->GetStringPointer());
		}

		HRESULT hrStatus;
		pResults->GetStatus(&hrStatus);
		if (FAILED(hrStatus))
		{
			LOG(LogAssetImporterMaterial, Error, "shader complation Failed.");
			return false;
		}

		Microsoft::WRL::ComPtr<IDxcBlob> pShader = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
		pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);

		D3DCreateBlob(pShader->GetBufferSize(), ByteBlob);
		memcpy((*ByteBlob)->GetBufferPointer(), pShader->GetBufferPointer(), pShader->GetBufferSize());

		return true;
	}

	void AssetImporterMaterial::UpdateMaterialParameterSlots( Material* MaterialAsset, const std::string& ShaderCode )
	{
		std::vector<MaterialIndexedTexture2DParameter> OldTexture2Ds = MaterialAsset->m_Texture2DSlots;
		MaterialAsset->m_Texture2DSlots.clear();

		std::vector<std::string> NamedTokens;
		FindNamedTokens(ShaderCode, "@TEX2D", NamedTokens);

		for (int i = 0; i < NamedTokens.size(); i++)
		{
			std::string Path = DEFAULT_TEXTURE_PATH;
			std::string& name = NamedTokens[i];

			for (const MaterialIndexedTexture2DParameter& OldTexture2D : OldTexture2Ds)
			{
				if (OldTexture2D.m_Name == name)
				{
					Path = OldTexture2D.m_Texture2D.GetPath();
				}
			}

			MaterialAsset->m_Texture2DSlots.push_back(MaterialIndexedTexture2DParameter(name, Path, i));
		}

// ----------------------------------------------------------------------------------------------------------------------------

		std::vector<MaterialIndexedTextureCubeParameter> OldTextureCubes = MaterialAsset->m_TextureCubeSlots;
		MaterialAsset->m_TextureCubeSlots.clear();

		NamedTokens.clear();
		FindNamedTokens(ShaderCode, "@TEXCUBE", NamedTokens);

		for (int i = 0; i < NamedTokens.size(); i++)
		{
			std::string Path = "NULL";
			std::string& name = NamedTokens[i];

			for (const MaterialIndexedTextureCubeParameter& OldTextureCube : OldTextureCubes)
			{
				if (OldTextureCube.m_Name == name)
				{
					Path = OldTextureCube.m_TextureCube.GetPath();
				}
			}

			MaterialAsset->m_TextureCubeSlots.push_back(MaterialIndexedTextureCubeParameter(name, Path, i + MaterialAsset->m_Texture2DSlots.size()));
		}

// ----------------------------------------------------------------------------------------------------------------------------

		std::vector<MaterialIndexedFloatParameter> OldScalars = MaterialAsset->m_FloatSlots;
		MaterialAsset->m_FloatSlots.clear();

		NamedTokens.clear();
		FindNamedTokens(ShaderCode, "@SCALAR", NamedTokens);

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
		FindNamedTokens(ShaderCode, "@VECTOR", NamedTokens);

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

			MaterialAsset->m_Vector4Slots.push_back(MaterialIndexedVector4Parameter(name, Value, i * 4));
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