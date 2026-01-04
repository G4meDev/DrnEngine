#include "DrnPCH.h"
#include "Material.h"
#include "Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"
#include "Runtime/Renderer/MaterialPipelines.h"

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

			MaterialParameters.Serialize(Ar);

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
		}

		else
		{
			Ar << m_SourcePath; 
			
			m_MainShaderBlob.Serialize(Ar);
			m_HitProxyShaderBlob.Serialize(Ar);

			Ar << static_cast<uint8>(m_MaterialDomain);
			Ar << m_TwoSided;

			MaterialParameters.Serialize(Ar);

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
		m_MaterialPipelines = nullptr;
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

	void Material::UploadResources( D3D12CommandList* CommandList )
	{
		if (IsRenderStateDirty())
		{
			SCOPE_STAT();

			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			for (Texture2DProperty& Slot : MaterialParameters.m_Texture2DSlots)
			{
				if (!Slot.m_Texture2D.IsValid())
				{
					Slot.m_Texture2D.LoadChecked();
				}
			}
			
			for (TextureCubeProperty& Slot : MaterialParameters.m_TextureCubeSlots)
			{
				if (!Slot.m_TextureCube.IsValid())
				{
					Slot.m_TextureCube.LoadChecked();
				}
			}

			ReleasePSOs();

			m_MaterialPipelines = new MaterialPipelines(CommandList, this);

			ClearRenderStateDirty();
		}

		MaterialParameters.UploadResources(CommandList);
	}

	void Material::BindMainPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_MainPassPSO);

		BindResources(CommandList);
	}

	void Material::BindPrePass( D3D12CommandList* CommandList )
	{
		TRefCountPtr<GraphicsPipelineState> PSO;
		if (m_HasCustomPrePass)
		{
			PSO = m_MaterialPipelines->m_PrePassPSO;
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
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_PointLightShadowDepthPassPSO);
		
		BindResources(CommandList);
	}

	void Material::BindSpotLightShadowDepthPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_SpotLightShadowDepthPassPSO);
		
		BindResources(CommandList);
	}

	void Material::BindDeferredDecalPass( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_DeferredDecalPassPSO);

		BindResources(CommandList);
	}

	void Material::BindStaticMeshDecalPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_StaticMeshDecalPassPSO);

		BindResources(CommandList);
	}

#if WITH_EDITOR
	void Material::BindEditorPrimitivePass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_EditorProxyPSO);

		BindResources(CommandList);
	}

	void Material::BindSelectionPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_SelectionPassPSO);
		CommandList->GetD3D12CommandList()->OMSetStencilRef( 255 );

		BindResources(CommandList);
	}

	void Material::BindHitProxyPass( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_HitProxyPassPSO);

		BindResources(CommandList);
	}

#endif

	void Material::BindResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicRootConstant(MaterialParameters.ParametersBuffer ? MaterialParameters.ParametersBuffer->GetViewIndex() : 0, 3);
	}

	void Material::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		MaterialParameters.SetNamedTexture2D(Name, TextureAsset);
	}

	void Material::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		MaterialParameters.SetIndexedTexture2D(Index, TextureAsset);
	}

	void Material::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		MaterialParameters.SetNamedTextureCube(Name, TextureAsset);
	}

	void Material::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		MaterialParameters.SetIndexedTextureCube(Index, TextureAsset);
	}

	void Material::SetIndexedScalar( uint32 Index, float Value )
	{
		MaterialParameters.SetIndexedScalar(Index, Value);
	}

	void Material::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		MaterialParameters.SetIndexedVector(Index, Value);
	}

	void Material::SetNamedScalar( const std::string& Name, float Value )
	{
		MaterialParameters.SetNamedScalar(Name, Value);
	}

	void Material::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		MaterialParameters.SetNamedVector4(Name, Value);
	}



        }  // namespace Drn