#include "DrnPCH.h"
#include "AssetImporterMaterial.h"

#if WITH_EDITOR

#include <dxcapi.h>

LOG_DEFINE_CATEGORY( LogAssetImporterMaterial, "AssetImporterMaterial" );

namespace Drn
{
	enum class EMaterialShaderFlag : uint32
	{
		DomainSurface,
		DomainDecal,

		BlendOpaque,
		BlendMasked,
		BlendTranslucent,

		ShadingLit,
		ShadingUnlit,

		TwoSided,

		VertexFactoryStaticMesh,
		VertexFactoryInstancedStaticMesh,
		VertexFactoryDecal,

		HasPrePass,
		HasCustomPrePass,
		HasShadowPass,
		HasMainPass,
		HasHitProxyPass,
		HasEditorPrimitivePass,
		HasEditorSelectionPass,
		HasDecalPass,
		HasVeloictyPass,
	};
	
	const std::unordered_map<EMaterialShaderFlag, std::string> MaterialShaderFlagTokenMap = 
	{
		{EMaterialShaderFlag::DomainSurface						, "DOMAIN_SURFACE"},
		{EMaterialShaderFlag::DomainDecal						, "DOMAIN_DECAL"},

		{EMaterialShaderFlag::BlendOpaque						, "BLEND_OPAQUE"},
		{EMaterialShaderFlag::BlendMasked						, "BLEND_MASKED"},
		{EMaterialShaderFlag::BlendTranslucent					, "BLEND_TRANSLUCENT"},

		{EMaterialShaderFlag::ShadingLit						, "SHADING_LIT"},
		{EMaterialShaderFlag::ShadingUnlit						, "SHADING_UNLIT"},

		{EMaterialShaderFlag::TwoSided							, "TWO_SIDED"},

		{EMaterialShaderFlag::VertexFactoryStaticMesh			, "SUPPORT_STATICMESH"},
		{EMaterialShaderFlag::VertexFactoryInstancedStaticMesh	, "SUPPORT_INSTANCED"},
		{EMaterialShaderFlag::VertexFactoryDecal				, "SUPPORT_DECAL"},

		{EMaterialShaderFlag::HasPrePass						, "SUPPORT_PRE_PASS"},
		{EMaterialShaderFlag::HasCustomPrePass					, "HAS_CUSTOM_PRE_PASS"},
		{EMaterialShaderFlag::HasShadowPass						, "SUPPORT_SHADOW_PASS"},
		{EMaterialShaderFlag::HasMainPass						, "SUPPORT_MAIN_PASS"},
		{EMaterialShaderFlag::HasHitProxyPass					, "SUPPORT_HIT_PROXY_PASS"},
		{EMaterialShaderFlag::HasEditorPrimitivePass			, "SUPPORT_EDITOR_PRIMITIVE_PASS"},
		{EMaterialShaderFlag::HasEditorSelectionPass			, "SUPPORT_EDITOR_SELECTION_PASS"},
		{EMaterialShaderFlag::HasDecalPass						, "SUPPORT_DECAL_PASS"},
		{EMaterialShaderFlag::HasVeloictyPass					, "SUPPORT_VELOCITY"},
	};

	inline static bool IsMaterialFlagDomain( EMaterialShaderFlag Flag )
	{
		return Flag == EMaterialShaderFlag::DomainSurface
			|| Flag == EMaterialShaderFlag::DomainDecal;
	}

	inline static bool IsMaterialFlagBlendMode( EMaterialShaderFlag Flag )
	{
		return Flag == EMaterialShaderFlag::BlendOpaque
			|| Flag == EMaterialShaderFlag::BlendMasked
			|| Flag == EMaterialShaderFlag::BlendTranslucent;
	}

	inline static bool IsMaterialFlagShadingModel( EMaterialShaderFlag Flag )
	{
		return Flag == EMaterialShaderFlag::ShadingLit
			|| Flag == EMaterialShaderFlag::ShadingUnlit;
	}

	EMaterialShaderFlag GetShaderFlagFromVertexFactory(VertexFactoryType* VertexFactory)
	{
		if (VertexFactory == VertexFactoryType::StaticMesh)
		{
			return EMaterialShaderFlag::VertexFactoryStaticMesh;
		}

		else if (VertexFactory == VertexFactoryType::InstancedStaticMesh)
		{
			return EMaterialShaderFlag::VertexFactoryInstancedStaticMesh;
		}

		else if (VertexFactory == VertexFactoryType::Decal)
		{
			return EMaterialShaderFlag::VertexFactoryDecal;
		}

		drn_check(false);
		return EMaterialShaderFlag::VertexFactoryStaticMesh;
	}

	struct MaterialShaderFlags
	{
		std::vector<EMaterialShaderFlag> Flags;

		void PushFlag(EMaterialShaderFlag Flag)
		{
			auto It = std::find(Flags.begin(), Flags.end(), Flag);
			if (It == Flags.end())
			{
				Flags.push_back(Flag);
			}
			else
			{
				drn_check(false); // adding already existing flag
			}
		}

		void ValidateFlags()
		{
			int32 DomainFlagCount = 0;
			for (EMaterialShaderFlag Flag : Flags)
			{
				if (IsMaterialFlagDomain(Flag))
				{
					DomainFlagCount++;
				}
			}
			drn_check(DomainFlagCount == 1);

			int32 BlendModeFlagCount = 0;
			for (EMaterialShaderFlag Flag : Flags)
			{
				if (IsMaterialFlagBlendMode(Flag))
				{
					BlendModeFlagCount++;
				}
			}
			drn_check(BlendModeFlagCount == 1);
			
			int32 ShadingModelFlagCount = 0;
			for (EMaterialShaderFlag Flag : Flags)
			{
				if (IsMaterialFlagShadingModel(Flag))
				{
					ShadingModelFlagCount++;
				}
			}
			drn_check(ShadingModelFlagCount == 1);
		}

		bool HasFlag(EMaterialShaderFlag Flag)
		{
			for (EMaterialShaderFlag ShaderFlag : Flags)
			{
				if (ShaderFlag == Flag)
				{
					return true;
				}
			}

			return false;
		}

		EMaterialDomain GetMaterialDomain()
		{
			if (HasFlag(EMaterialShaderFlag::DomainSurface))
			{
				return EMaterialDomain::Surface;
			}

			else if (HasFlag(EMaterialShaderFlag::DomainDecal))
			{
				return EMaterialDomain::Decal;
			}

			drn_check(false);
			return EMaterialDomain::Surface;
		}

		EBlendMode GetMaterialBlendMode()
		{
			if (HasFlag(EMaterialShaderFlag::BlendOpaque))
			{
				return EBlendMode::Opaque;
			}

			else if (HasFlag(EMaterialShaderFlag::BlendMasked))
			{
				return EBlendMode::Masked;
			}

			else if (HasFlag(EMaterialShaderFlag::BlendTranslucent))
			{
				return EBlendMode::Translucent;
			}

			drn_check(false);
			return EBlendMode::Opaque;
		}

		EMaterialShadingModel GetMaterialShadingModel()
		{
			if (HasFlag(EMaterialShaderFlag::ShadingLit))
			{
				return EMaterialShadingModel::Lit;
			}

			else if (HasFlag(EMaterialShaderFlag::ShadingUnlit))
			{
				return EMaterialShadingModel::Unlit;
			}

			drn_check(false);
			return EMaterialShadingModel::Lit;
		}
	};

// -----------------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------------------------------

		MaterialShaderFlags Flags;
		for (auto& It : MaterialShaderFlagTokenMap)
		{
			if (ShaderString.find(It.second) != std::string::npos)
			{
				Flags.PushFlag(It.first);
			}
		}
		Flags.ValidateFlags();

		std::vector<VertexFactoryType*> SupportedVertexFactories;
		for (VertexFactoryType* VertexFactory : VertexFactoryType::GlobalFactories)
		{
			EMaterialShaderFlag FactoryFlag = GetShaderFlagFromVertexFactory(VertexFactory);
			if (Flags.HasFlag(FactoryFlag))
			{
				SupportedVertexFactories.push_back(VertexFactory);
			}
		}

		const EMaterialDomain MaterialDomain = Flags.GetMaterialDomain();
		const EBlendMode BlendMode = Flags.GetMaterialBlendMode();
		const EMaterialShadingModel ShadingModel = Flags.GetMaterialShadingModel();
		const bool bMasked = Flags.HasFlag(EMaterialShaderFlag::BlendMasked);
		const bool bTwoSided = Flags.HasFlag(EMaterialShaderFlag::TwoSided);

		bool Successed = true;

		auto CompileShaderBlobConditional = [&](bool Condition, const std::string& InPath,
			const wchar_t* InEntryPoint, const wchar_t* InProfile, const std::vector<const wchar_t*>& Macros, ID3DBlob** InByteBlob) 
		{
			if (Condition)
			{
				Successed &= CompileShader( StringHelper::s2ws(InPath), InEntryPoint, InProfile, Macros, InByteBlob);
			}
		};

		MaterialShaders Shaders;
		if (MaterialDomain == EMaterialDomain::Surface)
		{
			for (VertexFactoryType* VertexFactory : SupportedVertexFactories)
			{
				if (Flags.HasFlag(EMaterialShaderFlag::HasPrePass) && Flags.HasFlag(EMaterialShaderFlag::HasCustomPrePass))
				{
					ShaderBlob CustomPrePassShaderBlob;
					std::vector<const wchar_t*> Macros = { L"PRE_PASS=1" };
					Macros.push_back(VertexFactory->GetShaderMacro());

					CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &CustomPrePassShaderBlob.m_VS);
					CompileShaderBlobConditional(bMasked && HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &CustomPrePassShaderBlob.m_PS);

					Shaders.PushShader(VertexFactory, EMaterialStage::Prepass, CustomPrePassShaderBlob);
				}

				if (Flags.HasFlag(EMaterialShaderFlag::HasShadowPass))
				{
					{
						ShaderBlob PointLightShadowDepthShaderBlob;
						std::vector<const wchar_t*> Macros = { L"SHADOW_PASS=1", L"SHADOW_PASS_POINTLIGHT=1" };
						Macros.push_back(VertexFactory->GetShaderMacro());
						CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &PointLightShadowDepthShaderBlob.m_VS);
						if (bMasked)
						{
							CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &PointLightShadowDepthShaderBlob.m_PS);
						}
						CompileShaderBlobConditional(true, Path, L"PointLightShadow_GS", L"gs_6_6", Macros, &PointLightShadowDepthShaderBlob.m_GS);
						CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", Macros, &PointLightShadowDepthShaderBlob.m_HS);
						CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", Macros, &PointLightShadowDepthShaderBlob.m_DS);

						Shaders.PushShader(VertexFactory, EMaterialStage::PointLightShadow, PointLightShadowDepthShaderBlob);
					}

					{
						ShaderBlob SpotLightShadowDepthShaderBlob;
						std::vector<const wchar_t*> Macros = { L"SHADOW_PASS=1", L"SHADOW_PASS_SPOTLIGHT=1" };
						Macros.push_back(VertexFactory->GetShaderMacro());
						CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &SpotLightShadowDepthShaderBlob.m_VS);
						if (bMasked)
						{
							CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &SpotLightShadowDepthShaderBlob.m_PS);
						}
						//CompileShaderBlobConditional(true, Path, L"PointLightShadow_GS", L"gs_6_6", SpotlLightShadowMacros, &SpotLightShadowDepthShaderBlob.m_GS);
						CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", Macros, &SpotLightShadowDepthShaderBlob.m_HS);
						CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", Macros, &SpotLightShadowDepthShaderBlob.m_DS);

						Shaders.PushShader(VertexFactory, EMaterialStage::SpotLightShadow, SpotLightShadowDepthShaderBlob);
					}
				}

				if (Flags.HasFlag(EMaterialShaderFlag::HasMainPass))
				{
					ShaderBlob MainShaderBlob;
					std::vector<const wchar_t*> Macros = { L"MAIN_PASS=1" };
					Macros.push_back(VertexFactory->GetShaderMacro());

					CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &MainShaderBlob.m_VS);
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &MainShaderBlob.m_PS);
					CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", Macros, &MainShaderBlob.m_GS);
					CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", Macros, &MainShaderBlob.m_HS);
					CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", Macros, &MainShaderBlob.m_DS);

					Shaders.PushShader(VertexFactory, EMaterialStage::Main, MainShaderBlob);
				}

				if (Flags.HasFlag(EMaterialShaderFlag::HasVeloictyPass))
				{
					ShaderBlob VelocityShaderBlob;
					std::vector<const wchar_t*> Macros = { L"VELOCITY_PASS=1" };
					Macros.push_back(VertexFactory->GetShaderMacro());

					CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &VelocityShaderBlob.m_VS);
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &VelocityShaderBlob.m_PS);
					//CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", Macros, &MainShaderBlob.m_GS);
					//CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", Macros, &MainShaderBlob.m_HS);
					//CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", Macros, &MainShaderBlob.m_DS);

					Shaders.PushShader(VertexFactory, EMaterialStage::Velocity, VelocityShaderBlob);
				}

				if (Flags.HasFlag(EMaterialShaderFlag::HasHitProxyPass))
				{
					ShaderBlob HitProxyShaderBlob;
					std::vector<const wchar_t*> Macros = { L"HITPROXY_PASS=1" };
					Macros.push_back(VertexFactory->GetShaderMacro());

					CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &HitProxyShaderBlob.m_VS);
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &HitProxyShaderBlob.m_PS);
					CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", Macros, &HitProxyShaderBlob.m_GS);
					CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", Macros, &HitProxyShaderBlob.m_HS);
					CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", Macros, &HitProxyShaderBlob.m_DS);

					Shaders.PushShader(VertexFactory, EMaterialStage::Hitproxy, HitProxyShaderBlob);
				}

				if (Flags.HasFlag(EMaterialShaderFlag::HasEditorPrimitivePass))
				{
					ShaderBlob EditorPrimitiveShaderBlob;
					std::vector<const wchar_t*> Macros = { L"EDITOR_PRIMITIVE_PASS=1" };
					Macros.push_back(VertexFactory->GetShaderMacro());

					CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &EditorPrimitiveShaderBlob.m_VS);
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &EditorPrimitiveShaderBlob.m_PS);
					CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", Macros, &EditorPrimitiveShaderBlob.m_GS);
					CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", Macros, &EditorPrimitiveShaderBlob.m_HS);
					CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", Macros, &EditorPrimitiveShaderBlob.m_DS);

					Shaders.PushShader(VertexFactory, EMaterialStage::EditorPrimitive, EditorPrimitiveShaderBlob);
				}

				if (Flags.HasFlag(EMaterialShaderFlag::HasEditorSelectionPass))
				{
					drn_check(Flags.HasFlag(EMaterialShaderFlag::HasMainPass));

					ShaderBlob MainShaderBlob;
					std::vector<const wchar_t*> Macros = { L"MAIN_PASS=1" };
					Macros.push_back(VertexFactory->GetShaderMacro());

					CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &MainShaderBlob.m_VS);
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &MainShaderBlob.m_PS);
					CompileShaderBlobConditional(HasGS, Path, L"Main_GS", L"gs_6_6", Macros, &MainShaderBlob.m_GS);
					CompileShaderBlobConditional(HasHS, Path, L"Main_HS", L"hs_6_6", Macros, &MainShaderBlob.m_HS);
					CompileShaderBlobConditional(HasDS, Path, L"Main_DS", L"ds_6_6", Macros, &MainShaderBlob.m_DS);

					Shaders.PushShader(VertexFactory, EMaterialStage::EditorSelection, MainShaderBlob);
				}

				if (BlendMode == EBlendMode::Translucent)
				{
					ShaderBlob TranslucentShaderBlob;
					std::vector<const wchar_t*> Macros = { L"TRANSLUCENCY_PASS=1" };
					Macros.push_back(VertexFactory->GetShaderMacro());

					CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &TranslucentShaderBlob.m_VS);
					CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &TranslucentShaderBlob.m_PS);

					Shaders.PushShader(VertexFactory, EMaterialStage::Translucensy, TranslucentShaderBlob);
				}
			}
		}

		else if (MaterialDomain == EMaterialDomain::Decal)
		{
			for (VertexFactoryType* VertexFactory : SupportedVertexFactories)
			{
				drn_check(VertexFactory == VertexFactoryType::StaticMesh || VertexFactory == VertexFactoryType::Decal);

				ShaderBlob ShaderBlob;
				std::vector<const wchar_t*> Macros = { L"DECAL_PASS=1" };
				Macros.push_back(VertexFactory->GetShaderMacro());

				CompileShaderBlobConditional(HasVS, Path, L"Main_VS", L"vs_6_6", Macros, &ShaderBlob.m_VS);
				CompileShaderBlobConditional(HasPS, Path, L"Main_PS", L"ps_6_6", Macros, &ShaderBlob.m_PS);

				Shaders.PushShader(VertexFactory, EMaterialStage::Decal, ShaderBlob);
			}
		}

// ------------------------------------------------------------------------------------------------------------------------

		if (Successed)
		{
			MaterialAsset->ShaderParameters.MaterialDomain = MaterialDomain;
			MaterialAsset->ShaderParameters.BlendMode = BlendMode;
			MaterialAsset->ShaderParameters.ShadingModel = ShadingModel;
			MaterialAsset->ShaderParameters.bIsTwoSided = bTwoSided;

			MaterialAsset->Shaders = Shaders;

			MaterialAsset->ShaderParameters.bHasPrepass = Flags.HasFlag(EMaterialShaderFlag::HasPrePass);
			MaterialAsset->ShaderParameters.bHasCustomPrepass = Flags.HasFlag(EMaterialShaderFlag::HasCustomPrePass);
			MaterialAsset->ShaderParameters.bHasShadowPass = Flags.HasFlag(EMaterialShaderFlag::HasShadowPass);
			MaterialAsset->ShaderParameters.bHasMainPass = Flags.HasFlag(EMaterialShaderFlag::HasMainPass);
			MaterialAsset->ShaderParameters.bHasHitProxyPass = Flags.HasFlag(EMaterialShaderFlag::HasHitProxyPass);
			MaterialAsset->ShaderParameters.bHasEditorPrimitivePass = Flags.HasFlag(EMaterialShaderFlag::HasEditorPrimitivePass);
			MaterialAsset->ShaderParameters.bHasEditorSelectionPass = Flags.HasFlag(EMaterialShaderFlag::HasEditorSelectionPass);
			MaterialAsset->ShaderParameters.bHasDecalPass = Flags.HasFlag(EMaterialShaderFlag::HasDecalPass);
			MaterialAsset->ShaderParameters.bHasVelocityPass = Flags.HasFlag(EMaterialShaderFlag::HasVeloictyPass);
			MaterialAsset->ShaderParameters.bHasTranslucencyPass = (MaterialDomain == EMaterialDomain::Surface) && (BlendMode == EBlendMode::Translucent);

			MaterialAsset->ShaderParameters.bIsUsedWithStaticMesh = Flags.HasFlag(EMaterialShaderFlag::VertexFactoryStaticMesh);
			MaterialAsset->ShaderParameters.bIsUsedWithInstancedStaticMesh = Flags.HasFlag(EMaterialShaderFlag::VertexFactoryInstancedStaticMesh);
			MaterialAsset->ShaderParameters.bIsUsedWithDecal = Flags.HasFlag(EMaterialShaderFlag::VertexFactoryDecal);

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

		std::vector<Vector4Property> OldVector4s = MaterialAsset->MaterialParameters.m_Vector4Slots;
		std::vector<FloatProperty> OldFloats = MaterialAsset->MaterialParameters.m_FloatSlots;
		std::vector<Texture2DProperty> OldTexture2Ds = MaterialAsset->MaterialParameters.m_Texture2DSlots;
		std::vector<TextureCubeProperty> OldTextureCubes = MaterialAsset->MaterialParameters.m_TextureCubeSlots;
		MaterialAsset->MaterialParameters.Clear();

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

			MaterialAsset->MaterialParameters.m_Vector4Slots.push_back(Vector4Property(Param.Name, Value));
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

			MaterialAsset->MaterialParameters.m_FloatSlots.push_back(FloatProperty(Param.Name, Value));
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

			MaterialAsset->MaterialParameters.m_Texture2DSlots.push_back(Texture2DProperty(Param.Name, Path));
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

			MaterialAsset->MaterialParameters.m_TextureCubeSlots.push_back(TextureCubeProperty(Param.Name, Path));
		}
	}

	void AssetImporterMaterial::Import( MaterialInstance* MaterialAsset, const std::string& Path )
	{
		AssetHandle<Material> ParentMaterial(Path);
		ParentMaterial.LoadChecked();
		drn_check(ParentMaterial.IsValid());

		MaterialUniformParameters OldParams = MaterialAsset->MaterialParameters;
		MaterialAsset->MaterialParameters.CopyParameters(ParentMaterial->MaterialParameters);
		MaterialAsset->MaterialParameters.OverrideParams(OldParams);
	}
}

#endif