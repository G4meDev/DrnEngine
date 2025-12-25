#include "DrnPCH.h"
#include "Material.h"
#include "Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"

LOG_DEFINE_CATEGORY( LogMaterial, "Material" );

namespace Drn
{
	Material::Material( const std::string& InPath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
		, m_SupportMainPass(true)
		, m_SupportPrePass(true)
		, m_SupportHitProxyPass(false)
		, m_SupportEditorPrimitivePass(false)
		, m_SupportEditorSelectionPass(true)
		, m_SupportDeferredDecalPass(false)
		, m_SupportStaticMeshDecalPass(false)
		, m_MaterialDomain(EMaterialDomain::Surface)
		, m_TwoSided(false)
		, m_ScalarBufferDirty(true)
		, m_VectorBufferDirty(true)
		, m_TextureBufferDirty(true)
	{
		Load();
	}

#if WITH_EDITOR
	Material::Material( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
		, m_SupportMainPass(true)
		, m_SupportPrePass(true)
		, m_SupportHitProxyPass(false)
		, m_SupportEditorPrimitivePass(false)
		, m_SupportEditorSelectionPass(true)
		, m_SupportDeferredDecalPass(false)
		, m_SupportStaticMeshDecalPass(false)
		, m_MaterialDomain(EMaterialDomain::Surface)
		, m_TwoSided(false)
		, m_ScalarBufferDirty(true)
		, m_VectorBufferDirty(true)
		, m_TextureBufferDirty(true)
	{
		m_SourcePath = InSourcePath;
		Import();
	}
#endif
	
	Material::~Material()
	{
		ReleasePSOs();
		
	}

	EAssetType Material::GetAssetType() { return EAssetType::Material; }

	void Material::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			ReleaseShaderBlobs();

			Ar >> m_SourcePath;

			m_MainShaderBlob.Serialize(Ar);
			m_HitProxyShaderBlob.Serialize(Ar);

			Ar >> *((uint8*)(&m_MaterialDomain));
			Ar >> m_TwoSided;

			uint8 Texture2DCount;
			Ar >> Texture2DCount;
			m_Texture2DSlots.clear();
			m_Texture2DSlots.reserve(Texture2DCount);
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots.push_back(MaterialIndexedTexture2DParameter());
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount;
			Ar >> TextureCubeCount;
			m_TextureCubeSlots.clear();
			m_TextureCubeSlots.reserve(TextureCubeCount);
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots.push_back(MaterialIndexedTextureCubeParameter());
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount;
			Ar >> ScalarCount;
			m_FloatSlots.clear();
			m_FloatSlots.reserve(ScalarCount);
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots.push_back(MaterialIndexedFloatParameter());
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count;
			Ar >> Vector4Count;
			m_Vector4Slots.clear();
			m_Vector4Slots.reserve(Vector4Count);
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots.push_back(MaterialIndexedVector4Parameter());
				m_Vector4Slots[i].Serialize(Ar);
			}

			Ar >> m_SupportHitProxyPass;
			Ar >> m_SupportMainPass;
			Ar >> m_SupportEditorPrimitivePass;

			m_EditorPrimitiveShaderBlob.Serialize(Ar);

			Ar >> m_SupportEditorSelectionPass;

			Ar >> m_SupportShadowPass;
			m_PointlightShadowDepthShaderBlob.Serialize(Ar);
			m_SpotlightShadowDepthShaderBlob.Serialize(Ar);

			Ar >> m_SupportDeferredDecalPass;
			m_DeferredDecalShaderBlob.Serialize(Ar);

			Ar >> m_SupportStaticMeshDecalPass;
			m_StaticMeshDecalShaderBlob.Serialize(Ar);

			Ar >> m_SupportPrePass;
			Ar >> m_HasCustomPrePass;
			m_PrePassShaderBlob.Serialize(Ar);

			InitalizeParameterMap();
		}

		else
		{
			Ar << m_SourcePath; 
			
			m_MainShaderBlob.Serialize(Ar);
			m_HitProxyShaderBlob.Serialize(Ar);

			Ar << static_cast<uint8>(m_MaterialDomain);
			Ar << m_TwoSided;

			uint8 Texture2DCount = m_Texture2DSlots.size();
			Ar << Texture2DCount;
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount = m_TextureCubeSlots.size();
			Ar << TextureCubeCount;
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount = m_FloatSlots.size();
			Ar << ScalarCount;
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count = m_Vector4Slots.size();
			Ar << Vector4Count;
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots[i].Serialize(Ar);
			}

			Ar << m_SupportHitProxyPass;
			Ar << m_SupportMainPass;
			Ar << m_SupportEditorPrimitivePass;
			m_EditorPrimitiveShaderBlob.Serialize(Ar);
			Ar << m_SupportEditorSelectionPass;

			Ar << m_SupportShadowPass;
			m_PointlightShadowDepthShaderBlob.Serialize(Ar);
			m_SpotlightShadowDepthShaderBlob.Serialize(Ar);

			Ar << m_SupportDeferredDecalPass;
			m_DeferredDecalShaderBlob.Serialize(Ar);

			Ar << m_SupportStaticMeshDecalPass;
			m_StaticMeshDecalShaderBlob.Serialize(Ar);

			Ar << m_SupportPrePass;
			Ar << m_HasCustomPrePass;
			m_PrePassShaderBlob.Serialize(Ar);
		}
	}

	void Material::ReleaseShaderBlobs()
	{
		m_MainShaderBlob.ReleaseBlobs();
		m_PrePassShaderBlob.ReleaseBlobs();
		m_HitProxyShaderBlob.ReleaseBlobs();
		m_EditorPrimitiveShaderBlob.ReleaseBlobs();
		m_PointlightShadowDepthShaderBlob.ReleaseBlobs();
		m_SpotlightShadowDepthShaderBlob.ReleaseBlobs();
		m_DeferredDecalShaderBlob.ReleaseBlobs();
		m_StaticMeshDecalShaderBlob.ReleaseBlobs();
	}

	void Material::ReleasePSOs()
	{
		m_MainPassPSO = nullptr;
		m_PrePassPSO = nullptr;
		m_PointLightShadowDepthPassPSO = nullptr;
		m_SpotLightShadowDepthPassPSO = nullptr;
		m_DeferredDecalPassPSO = nullptr;
		m_StaticMeshDecalPassPSO = nullptr;

#if WITH_EDITOR
		m_SelectionPassPSO = nullptr;
		m_HitProxyPassPSO = nullptr;
		m_HitProxyPassPSO = nullptr;
#endif
	}

	void Material::InitalizeParameterMap()
	{
		m_ScalarMap.clear();
		m_Vector4Map.clear();

		for ( int i = 0; i < m_FloatSlots.size(); i++ )
		{
			m_ScalarMap[m_FloatSlots[i].m_Name] = &m_FloatSlots[i];
		}

		for ( int i = 0; i < m_Vector4Slots.size(); i++ )
		{
			m_Vector4Map[m_Vector4Slots[i].m_Name] = &m_Vector4Slots[i];
		}
	}

#if WITH_EDITOR
	void Material::Import()
	{
		//Renderer::Get()->Flush();
		AssetImporterMaterial::Import( this, m_SourcePath );
		Save();
		Load();

		MarkRenderStateDirty();
	}


	void Material::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewMaterialGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void Material::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}
#endif

// ---------------------------------------------------------------------------------------------------------------

	static BoundShaderStateInput GetShaderStateInput(VertexDeclaration* VDeclaration, ShaderBlob& Blob)
	{
		VertexShader* VShader = nullptr;
		HullShader* HShader = nullptr;
		DomainShader* DShader = nullptr;
		PixelShader* PShader = nullptr;
		GeometryShader* GShader = nullptr;

		if (Blob.m_VS)
		{
			VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = Blob.m_VS->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = Blob.m_VS->GetBufferSize();
		}

		if (Blob.m_HS)
		{
			HShader = new HullShader();
			HShader->ByteCode.pShaderBytecode = Blob.m_HS->GetBufferPointer();
			HShader->ByteCode.BytecodeLength = Blob.m_HS->GetBufferSize();
		}

		if (Blob.m_DS)
		{
			DShader = new DomainShader();
			DShader->ByteCode.pShaderBytecode = Blob.m_DS->GetBufferPointer();
			DShader->ByteCode.BytecodeLength = Blob.m_DS->GetBufferSize();
		}

		if (Blob.m_PS)
		{
			PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = Blob.m_PS->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = Blob.m_PS->GetBufferSize();
		}

		if (Blob.m_GS)
		{
			GShader = new GeometryShader();
			GShader->ByteCode.pShaderBytecode = Blob.m_GS->GetBufferPointer();
			GShader->ByteCode.BytecodeLength = Blob.m_GS->GetBufferSize();
		}

		return BoundShaderStateInput(VDeclaration, VShader, HShader, DShader, PShader, GShader);
	}

	void Material::UploadResources( D3D12CommandList* CommandList )
	{
		if (IsRenderStateDirty())
		{
			SCOPE_STAT();

			//InitalizeParameterMap();

			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			for (Texture2DProperty& Slot : m_Texture2DSlots)
			{
				if (!Slot.m_Texture2D.IsValid())
				{
					Slot.m_Texture2D.LoadChecked();
				}
			}

			for (TextureCubeProperty& Slot : m_TextureCubeSlots)
			{
				if (!Slot.m_TextureCube.IsValid())
				{
					Slot.m_TextureCube.LoadChecked();
				}
			}

			ReleasePSOs();

#if D3D12_Debug_INFO
			std::string name = Path::ConvertShortPath(m_Path);
			name = Path::RemoveFileExtension(name);
#endif

			const D3D12_CULL_MODE CullMode = GetCullMode();

			if (m_SupportMainPass)
			{
				BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_MainShaderBlob);
				TRefCountPtr<BlendState> BState = nullptr;

				RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
				TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

				DepthStencilStateInitializer DInit(false, ECompareFunction::GreaterEqual);
				TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
				DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_COLOR_DEFERRED_FORMAT, GBUFFER_BASE_COLOR_FORMAT, GBUFFER_WORLD_NORMAL_FORMAT, GBUFFER_MASKS_FORMAT, GBUFFER_MASKS_FORMAT, GBUFFER_VELOCITY_FORMAT };
				ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

				GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
					6, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

				m_MainPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
				SetName(m_MainPassPSO->PipelineState, "PSO_MainPass_" + name);
			}

			if (m_SupportPrePass && m_HasCustomPrePass)
			{
				BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_PrePassShaderBlob);
				TRefCountPtr<BlendState> BState = nullptr;

				RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
				TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

				DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
				TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
				DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
				ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

				GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
					0, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

				m_PrePassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
				SetName(m_PrePassPSO->PipelineState, "PSO_PrePass_" + name);
			}

			if (m_SupportDeferredDecalPass)
			{
				BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_Pos, m_DeferredDecalShaderBlob);

				BlendStateInitializer BInit( {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::InverseSourceAlpha)} );
				BInit.bUseIndependentRenderTargetBlendStates = false;
				TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

				RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::Front);
				TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

				DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
				TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
				DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DECAL_BASE_COLOR_FORMAT, DECAL_NORMAL_FORMAT, DECAL_MASKS_FORMAT };
				ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

				GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
					3, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

				m_DeferredDecalPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
				SetName(m_DeferredDecalPassPSO->PipelineState, "PSO_DecalPass_" + name);
			}

			if (m_SupportStaticMeshDecalPass)
			{
				BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_StaticMeshDecalShaderBlob);

				BlendStateInitializer BInit( {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::InverseSourceAlpha)} );
				BInit.bUseIndependentRenderTargetBlendStates = false;
				TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

				RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
				TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

				DepthStencilStateInitializer DInit(false, ECompareFunction::GreaterEqual);
				TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
				DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DECAL_BASE_COLOR_FORMAT, DECAL_NORMAL_FORMAT, DECAL_MASKS_FORMAT };
				ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

				GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
					3, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

				m_StaticMeshDecalPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
				SetName(m_StaticMeshDecalPassPSO->PipelineState, "PSO_StaticMeshDecalPass_" + name);
			}

			if (m_SupportShadowPass)
			{
				{
					BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_PointlightShadowDepthShaderBlob);
					TRefCountPtr<BlendState> BState = nullptr;

					RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
					TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

					DepthStencilStateInitializer DInit(true, ECompareFunction::LessEqual);
					TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
					DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
					ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

					GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
						0, TargetFormats, TargetFlags, DXGI_FORMAT_D16_UNORM, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

					m_PointLightShadowDepthPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
					SetName(m_PointLightShadowDepthPassPSO->PipelineState, "PSO_PointLightShadowDepthPass_" + name);
				}

				{
					BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_SpotlightShadowDepthShaderBlob);
					TRefCountPtr<BlendState> BState = nullptr;

					RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
					TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

					DepthStencilStateInitializer DInit(true, ECompareFunction::LessEqual);
					TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
					DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
					ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

					GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
						0, TargetFormats, TargetFlags, DXGI_FORMAT_D16_UNORM, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

					m_SpotLightShadowDepthPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
					SetName(m_SpotLightShadowDepthPassPSO->PipelineState, "PSO_SpotLightShadowDepthPass_" + name);
				}
			}

#if WITH_EDITOR

			if (m_SupportEditorSelectionPass)
			{
				BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_MainShaderBlob);
				BoundShaderState.m_PixelShader = nullptr;
				
				TRefCountPtr<BlendState> BState = nullptr;

				RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
				TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

				DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual,
					true, ECompareFunction::Always, EStencilOp::Replace, EStencilOp::Replace, EStencilOp::Replace,
					true, ECompareFunction::Always, EStencilOp::Replace, EStencilOp::Replace, EStencilOp::Replace);
				TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
				DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
				ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

				GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
					0, TargetFormats, TargetFlags, DEPTH_STENCIL_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

				m_SelectionPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
				SetName(m_SelectionPassPSO->PipelineState, "PSO_SelectionPass_" + name);
			}

			if (IsSupportingHitProxyPass())
			{
				BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_HitProxyShaderBlob);
				TRefCountPtr<BlendState> BState = nullptr;

				RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
				TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

				DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
				TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
				DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_GUID_FORMAT };
				ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

				GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
					1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

				m_HitProxyPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
				SetName(m_HitProxyPassPSO->PipelineState, "PSO_HitProxyPass_" + name);
			}

			if (m_SupportEditorPrimitivePass)
			{
				BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, m_EditorPrimitiveShaderBlob);
				TRefCountPtr<BlendState> BState = nullptr;

				RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
				TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

				DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
				TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
				DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
				ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

				GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
					1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

				m_EditorProxyPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
				SetName(m_EditorProxyPSO->PipelineState, "PSO_EditorPrimitive_" + name);
			}

#endif

			ClearRenderStateDirty();
		}

		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			//if (m_Texture2DSlots[i].m_Texture2D->IsRenderStateDirty())
			//{
			//	m_TextureBufferDirty = true;
			//}
			m_Texture2DSlots[i].m_Texture2D->UploadResources(CommandList);
		}

		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			if (m_TextureCubeSlots[i].m_TextureCube.IsValid())
			{
				//if (m_TextureCubeSlots[i].m_TextureCube->IsRenderStateDirty())
				//{
				//	m_TextureBufferDirty = true;
				//}
				m_TextureCubeSlots[i].m_TextureCube->UploadResources(CommandList);
			}
		}

		// if (m_TextureBufferDirty)
		if (true) //TODO: issue with re imported textures. cached keeps texture index of destroyed texture
		{
			m_TextureBufferDirty = false;
			TextureIndexBuffer = nullptr;

			std::vector<uint32> TextureIndices;
			TextureIndices.resize(m_Texture2DSlots.size() + m_TextureCubeSlots.size());

			for (auto& TextureSlot : m_Texture2DSlots)
			{
				TextureIndices[TextureSlot.m_Index] = TextureSlot.m_Texture2D.IsValid() ? TextureSlot.m_Texture2D->GetTextureIndex() : 0;
			}

			for (auto& TextureSlot : m_TextureCubeSlots)
			{
				TextureIndices[TextureSlot.m_Index] = TextureSlot.m_TextureCube.IsValid() ? TextureSlot.m_TextureCube->GetTextureIndex() : 0;
			}

			if (TextureIndices.size() > 0)
			{
				//uint32 Size = Align(TextureIndices.size() * sizeof(uint32), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
				uint32 Size = TextureIndices.size() * sizeof(uint32);
				TextureIndexBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), Size, EUniformBufferUsage::SingleFrame, TextureIndices.data());
			}
		}

		//if (m_ScalarBufferDirty)
		if (true)
		{
			m_ScalarBufferDirty = false;
			ScalarBuffer = nullptr;

			std::vector<float> Values;
			for (int i = 0; i < m_FloatSlots.size(); i++)
			{
				Values.push_back(m_FloatSlots[i].m_Value);
			}

			if (Values.size() > 0)
			{
				//uint32 Size = Align(Values.size() * sizeof(float), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
				uint32 Size = Values.size() * sizeof(float);
				ScalarBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), Size, EUniformBufferUsage::SingleFrame, Values.data());
			}
		}

		//if (m_VectorBufferDirty)
		if (true)
		{
			m_VectorBufferDirty = false;
			VectorBuffer = nullptr;

			std::vector<Vector4> Values;
			for (int i = 0; i < m_Vector4Slots.size(); i++)
			{
				Values.push_back(m_Vector4Slots[i].m_Value);
			}

			if (Values.size() > 0)
			{
				//uint32 Size = Align(Values.size() * sizeof(Vector4), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
				uint32 Size = Values.size() * sizeof(Vector4);
				VectorBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), Size, EUniformBufferUsage::SingleFrame, Values.data());
			}
		}
	}

	void Material::BindMainPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MainPassPSO);

		BindResources(CommandList);
	}

	void Material::BindPrePass( D3D12CommandList* CommandList )
	{
		TRefCountPtr<GraphicsPipelineState> PSO;
		if (m_HasCustomPrePass)
		{
			PSO = m_PrePassPSO;
		}
		
		else
		{
			PSO = m_TwoSided 
				? CommonResources::Get()->m_PositionOnlyDepthPSO->m_CullNonePSO
				: CommonResources::Get()->m_PositionOnlyDepthPSO->m_CullBackPSO;
		}

		if (!PSO)
		{
			return;
		}

		CommandList->SetGraphicPipelineState(PSO);

		BindResources(CommandList);
	}

	void Material::BindPointLightShadowDepthPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_PointLightShadowDepthPassPSO);
		
		BindResources(CommandList);
	}

	void Material::BindSpotLightShadowDepthPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_SpotLightShadowDepthPassPSO);
		
		BindResources(CommandList);
	}

	void Material::BindDeferredDecalPass( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicPipelineState(m_DeferredDecalPassPSO);

		BindResources(CommandList);
	}

	void Material::BindStaticMeshDecalPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_StaticMeshDecalPassPSO);

		BindResources(CommandList);
	}

#if WITH_EDITOR
	void Material::BindEditorPrimitivePass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_EditorProxyPSO);

		BindResources(CommandList);
	}

	void Material::BindSelectionPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_SelectionPassPSO);
		CommandList->GetD3D12CommandList()->OMSetStencilRef( 255 );

		BindResources(CommandList);
	}

	void Material::BindHitProxyPass( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicPipelineState(m_HitProxyPassPSO);

		BindResources(CommandList);
	}

#endif

	void Material::BindResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicRootConstant(TextureIndexBuffer ? TextureIndexBuffer->GetViewIndex() : 0, 3);
		CommandList->SetGraphicRootConstant(ScalarBuffer ? ScalarBuffer->GetViewIndex() : 0, 4);
		CommandList->SetGraphicRootConstant(VectorBuffer ? VectorBuffer->GetViewIndex() : 0, 5);
	}

	void Material::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			const Texture2DProperty& TextureSlot = m_Texture2DSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTexture2D(i, TextureAsset);
				return;
			}
		}
	}

	void Material::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_Texture2DSlots.size())
		{
			m_Texture2DSlots[Index].m_Texture2D = TextureAsset;
			m_TextureBufferDirty = true;
		}
	}

	void Material::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			const TextureCubeProperty& TextureSlot = m_TextureCubeSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTextureCube(i, TextureAsset);
				return;
			}
		}
	}

	void Material::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_TextureCubeSlots.size())
		{
			m_TextureCubeSlots[Index].m_TextureCube = TextureAsset;
			m_TextureBufferDirty = true;
		}
	}

	void Material::SetIndexedScalar( uint32 Index, float Value )
	{
		if (Index >= 0 && Index < m_FloatSlots.size())
		{
			m_FloatSlots[Index].m_Value = Value;
			m_ScalarBufferDirty = true;
		}
	}

	void Material::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		if (Index >= 0 && Index < m_Vector4Slots.size())
		{
			m_Vector4Slots[Index].m_Value = Value;
			m_VectorBufferDirty = true;
		}
	}

	void Material::SetNamedScalar( const std::string& Name, float Value )
	{
		auto it = m_ScalarMap.find(Name);

		if ( it != m_ScalarMap.end() )
		{
			SetIndexedScalar(it->second->m_Index, Value);
		}
	}

	void Material::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		auto it = m_Vector4Map.find(Name);
		
		if ( it != m_Vector4Map.end() )
		{
			SetIndexedVector(it->second->m_Index, Value);
		}
	}

}