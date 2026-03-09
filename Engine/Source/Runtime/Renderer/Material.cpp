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
		, MaterialName("InvalidMaterial")
	{
		Load();
	}

#if WITH_EDITOR
	Material::Material( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
		, MaterialName("InvalidMaterial")
	{
		m_SourcePath = InSourcePath;
		Import();
	}
#endif
	
	Material::~Material()
	{
		
	}

	EAssetType Material::GetAssetType() { return EAssetType::Material; }

	void Material::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_SourcePath;
			Ar >> ShaderParameters;
			Ar >> Shaders;

			MaterialParameters.Serialize(Ar);

			std::string name = Path::ConvertShortPath(m_Path);
			MaterialName = Path::RemoveFileExtension(name);
		}

		else
		{
			Ar << m_SourcePath; 
			Ar << ShaderParameters;
			Ar << Shaders;

			MaterialParameters.Serialize(Ar);
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

	bool Material::IsDependent( MaterialInterface* OtherMaterial ) const
	{
		return this == OtherMaterial;
	}

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

			Shaders.UploadPipelineStates(CommandList, this);

			ClearRenderStateDirty();
		}

		MaterialParameters.UploadResources(CommandList);
	}

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