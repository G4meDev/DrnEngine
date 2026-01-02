#include "DrnPCH.h"
#include "AssetImporterMaterial.h"

#if WITH_EDITOR

#include <dxcapi.h>

LOG_DEFINE_CATEGORY( LogAssetImporterMaterial, "AssetImporterMaterial" );

namespace Drn
{
	struct MaterialParamData
	{
		MaterialParamData() {}

		std::string Name;
	};

	struct VectorParamData : public MaterialParamData
	{
		VectorParamData() {}
	};

	struct ScalarParamData : public MaterialParamData
	{
		ScalarParamData() {}
	};

	struct Texture2DParamData : public MaterialParamData
	{
		Texture2DParamData() {}
	};

	struct TextureCubeParamData : public MaterialParamData
	{
		TextureCubeParamData() {}
	};

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
		bool SupportShadowPass = ShaderString.find( "SUPPORT_SHADOW_PASS" ) != std::string::npos;
		bool SupportDeferredDecalPass = ShaderString.find( "SUPPORT_DEFERRED_DECAL_PASS" ) != std::string::npos;
		bool SupportStaticMeshDecalPass = ShaderString.find( "SUPPORT_STATICMESH_DECAL_PASS" ) != std::string::npos;
		bool SupportPrePass = ShaderString.find( "SUPPORT_PRE_PASS" ) != std::string::npos;
		bool HasCustomPrePass = ShaderString.find( "HAS_CUSTOM_PRE_PASS" ) != std::string::npos;
		bool HasOpacity = ShaderString.find( "HAS_OPACITY" ) != std::string::npos;

		bool Successed = true;
		ShaderBlob MainShaderBlob;
		ShaderBlob HitProxyShaderBlob;
		ShaderBlob EditorPrimitiveShaderBlob;
		ShaderBlob PointLightShadowDepthShaderBlob;
		ShaderBlob SpotLightShadowDepthShaderBlob;
		ShaderBlob DeferredDecalShaderBlob;
		ShaderBlob StaticMeshDecalShaderBlob;
		ShaderBlob PrePassShaderBlob;

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
				if (HasOpacity)
				{
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_PS);
				}
				CompileShaderBlobConditional(true, Path, L"PointLightShadow_GS", L"gs_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_GS);
				CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_HS);
				CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", PointLightShadowMacros, &PointLightShadowDepthShaderBlob.m_DS);
			}

			{
				const std::vector<const wchar_t*> SpotlLightShadowMacros = { L"SHADOW_PASS=1", L"SHADOW_PASS_SPOTLIGHT=1" };
				CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_VS);
				if (HasOpacity)
				{
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_PS);
				}
				//CompileShaderBlobConditional(true, Path, L"PointLightShadow_GS", L"gs_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_GS);
				CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_HS);
				CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_DS);
			}
		}

		if (SupportDeferredDecalPass)
		{
			const std::vector<const wchar_t*> DeferredDecalMacros = { L"DEFERRED_DECAL_PASS=1" };

			CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", DeferredDecalMacros, &DeferredDecalShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", DeferredDecalMacros, &DeferredDecalShaderBlob.m_PS);
		}

		if (SupportStaticMeshDecalPass)
		{
			const std::vector<const wchar_t*> StaticMeshDecalMacros = { L"STATICMESH_DECAL_PASS=1" };

			CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", StaticMeshDecalMacros, &StaticMeshDecalShaderBlob.m_VS);
			CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", StaticMeshDecalMacros, &StaticMeshDecalShaderBlob.m_PS);
		}

		if (SupportPrePass && HasCustomPrePass)
		{
			const std::vector<const wchar_t*> PrePassMacros = { L"PRE_PASS=1" };

			CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", PrePassMacros, &PrePassShaderBlob.m_VS);

			if (HasOpacity)
			{
				CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", PrePassMacros, &PrePassShaderBlob.m_PS);
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
			MaterialAsset->m_DeferredDecalShaderBlob = DeferredDecalShaderBlob;
			MaterialAsset->m_StaticMeshDecalShaderBlob = StaticMeshDecalShaderBlob;
			MaterialAsset->m_PrePassShaderBlob = PrePassShaderBlob;

			MaterialAsset->m_SupportMainPass = SupportMainPass;
			MaterialAsset->m_SupportHitProxyPass = SupportHitProxyPass;
			MaterialAsset->m_SupportEditorPrimitivePass = SupportEditorPrimitivePass;
			MaterialAsset->m_SupportEditorSelectionPass = SupportEditorSelectionPass;
			MaterialAsset->m_SupportShadowPass = SupportShadowPass;
			MaterialAsset->m_SupportDeferredDecalPass = SupportDeferredDecalPass;
			MaterialAsset->m_SupportStaticMeshDecalPass = SupportStaticMeshDecalPass;
			MaterialAsset->m_SupportPrePass = SupportPrePass;
			MaterialAsset->m_HasCustomPrePass = HasCustomPrePass;

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

	void FindVectorParams(const std::string& ShaderCode, std::vector<VectorParamData>& Params)
	{
		std::istringstream iss(ShaderCode);
		for (std::string line; std::getline(iss, line); )
		{
			std::string str = " Mem(100)=120";
			std::regex regex("^[\\s]*VECTOR[\\s]*\\([\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*,[\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*\\)$");
			std::smatch m;

			if (std::regex_match(line, m, regex))
			{
				Params.push_back({});
				Params.back().Name = m[2];
			}
		}
	}

	void FindScalarParams(const std::string& ShaderCode, std::vector<ScalarParamData>& Params)
	{
		std::istringstream iss(ShaderCode);
		for (std::string line; std::getline(iss, line); )
		{
			std::string str = " Mem(100)=120";
			std::regex regex("^[\\s]*SCALAR[\\s]*\\([\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*,[\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*\\)$");
			std::smatch m;

			if (std::regex_match(line, m, regex))
			{
				Params.push_back({});
				Params.back().Name = m[2];
			}
		}
	}

	void FindTexture2DParams(const std::string& ShaderCode, std::vector<Texture2DParamData>& Params)
	{
		std::istringstream iss(ShaderCode);
		for (std::string line; std::getline(iss, line); )
		{
			std::string str = " Mem(100)=120";
			std::regex regex("^[\\s]*TEX2D[\\s]*\\([\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*,[\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*\\)$");
			std::smatch m;

			if (std::regex_match(line, m, regex))
			{
				Params.push_back({});
				Params.back().Name = m[2];
			}
		}
	}

	void FindTextureCubeParams(const std::string& ShaderCode, std::vector<TextureCubeParamData>& Params)
	{
		std::istringstream iss(ShaderCode);
		for (std::string line; std::getline(iss, line); )
		{
			std::string str = " Mem(100)=120";
			std::regex regex("^[\\s]*TEXCUBE[\\s]*\\([\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*,[\\s]*([_|A-Z|a-z]+[_|A-Z|a-z|0-9]+)[\\s]*\\)$");
			std::smatch m;

			if (std::regex_match(line, m, regex))
			{
				Params.push_back({});
				Params.back().Name = m[2];
			}
		}
	}

	void AssetImporterMaterial::UpdateMaterialParameterSlots( Material* MaterialAsset, const std::string& ShaderCode )
	{
		std::vector<VectorParamData> VectorParams;
		FindVectorParams(ShaderCode, VectorParams);

		std::vector<ScalarParamData> ScalarParams;
		FindScalarParams(ShaderCode, ScalarParams);

		std::vector<Texture2DParamData> Texture2DParams;
		FindTexture2DParams(ShaderCode, Texture2DParams);

		std::vector<TextureCubeParamData> TextureCubeParams;
		FindTextureCubeParams(ShaderCode, TextureCubeParams);

		std::vector<MaterialIndexedVector4Parameter> OldVector4s = MaterialAsset->m_Vector4Slots;
		MaterialAsset->m_Vector4Slots.clear();

		std::vector<MaterialIndexedFloatParameter> OldFloats = MaterialAsset->m_FloatSlots;
		MaterialAsset->m_FloatSlots.clear();

		std::vector<MaterialIndexedTexture2DParameter> OldTexture2Ds = MaterialAsset->m_Texture2DSlots;
		MaterialAsset->m_Texture2DSlots.clear();

		std::vector<MaterialIndexedTextureCubeParameter> OldTextureCubes = MaterialAsset->m_TextureCubeSlots;
		MaterialAsset->m_TextureCubeSlots.clear();

		for (int32 i = 0; i < VectorParams.size(); i++)
		{
			const VectorParamData& Param = VectorParams[i];
			Vector4 Value = 0.0f;

			for (const Vector4Property& OldVector4 : OldVector4s)
			{
				if (OldVector4.m_Name == Param.Name)
				{
					Value = OldVector4.m_Value;
				}
			}

			MaterialAsset->m_Vector4Slots.push_back(MaterialIndexedVector4Parameter(Param.Name, Value, i));
		}

		for (int32 i = 0; i < ScalarParams.size(); i++)
		{
			const ScalarParamData& Param = ScalarParams[i];
			float Value = 0.0f;

			for (const FloatProperty& OldFloat : OldFloats)
			{
				if (OldFloat.m_Name == Param.Name)
				{
					Value = OldFloat.m_Value;
				}
			}

			MaterialAsset->m_FloatSlots.push_back(MaterialIndexedFloatParameter(Param.Name, Value, i));
		}

		for (int32 i = 0; i < Texture2DParams.size(); i++)
		{
			const Texture2DParamData& Param = Texture2DParams[i];
			std::string Path = NAME_NULL;

			for (Texture2DProperty& OldTexture2D : OldTexture2Ds)
			{
				if (OldTexture2D.m_Name == Param.Name)
				{
					OldTexture2D.m_Texture2D.LoadChecked();
					if (OldTexture2D.m_Texture2D.IsValid())
					{
						Path = OldTexture2D.m_Texture2D.GetPath();
					}
				}
			}

			MaterialAsset->m_Texture2DSlots.push_back(MaterialIndexedTexture2DParameter(Param.Name, Path, i));
		}

		for (int32 i = 0; i < TextureCubeParams.size(); i++)
		{
			const TextureCubeParamData& Param = TextureCubeParams[i];
			std::string Path = NAME_NULL;

			for (TextureCubeProperty& OldTextureCube : OldTextureCubes)
			{
				if (OldTextureCube.m_Name == Param.Name)
				{
					OldTextureCube.m_TextureCube.LoadChecked();
					if (OldTextureCube.m_TextureCube.IsValid())
					{
						Path = OldTextureCube.m_TextureCube.GetPath();
					}
				}
			}

			MaterialAsset->m_TextureCubeSlots.push_back(MaterialIndexedTextureCubeParameter(Param.Name, Path, i));
		}
	}
}

#endif