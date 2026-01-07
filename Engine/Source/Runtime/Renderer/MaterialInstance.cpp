#include "DrnPCH.h"
#include "MaterialInstance.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"
#include "Editor/AssetPreview/AssetPreviewMaterialInstanceGuiLayer.h"

namespace Drn
{
	MaterialInstance::MaterialInstance( const std::string& InPath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
	{
		Load();

		Parent = AssetHandle<Material>(m_SourcePath);
		Parent.Load();
	}

#if WITH_EDITOR
	MaterialInstance::MaterialInstance( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
	{
		m_SourcePath = InSourcePath;
		Import();
		Parent = AssetHandle<Material>(m_SourcePath);
		Parent.Load();
	}
#endif

	MaterialInstance::~MaterialInstance()
	{
		
	}

	bool MaterialInstance::IsDependent( MaterialInterface* OtherMaterial ) const
	{
		return this == OtherMaterial || (Parent.Get() && Parent.Get() == OtherMaterial->GetMaterial());
	}

	void MaterialInstance::UploadResources( class D3D12CommandList* CommandList )
	{
		// TODO: just need to upload pipelinestates of parent
		Parent->UploadResources(CommandList);

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

			ClearRenderStateDirty();
		}

		MaterialParameters.UploadResources(CommandList);
	}

	void MaterialInstance::BindResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicRootConstant(MaterialParameters.ParametersBuffer ? MaterialParameters.ParametersBuffer->GetViewIndex() : 0, 3);
	}

	void MaterialInstance::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		MaterialParameters.SetNamedTexture2D(Name, TextureAsset);
	}

	void MaterialInstance::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		MaterialParameters.SetIndexedTexture2D(Index, TextureAsset);
	}

	void MaterialInstance::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		MaterialParameters.SetNamedTextureCube(Name, TextureAsset);
	}

	void MaterialInstance::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		MaterialParameters.SetIndexedTextureCube(Index, TextureAsset);
	}

	void MaterialInstance::SetIndexedScalar( uint32 Index, float Value )
	{
		MaterialParameters.SetIndexedScalar(Index, Value);
	}

	void MaterialInstance::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		MaterialParameters.SetIndexedVector(Index, Value);
	}

	void MaterialInstance::SetNamedScalar( const std::string& Name, float Value )
	{
		MaterialParameters.SetNamedScalar(Name, Value);
	}

	void MaterialInstance::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		MaterialParameters.SetNamedVector4(Name, Value);
	}

	void MaterialInstance::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_SourcePath;
			MaterialParameters.Serialize(Ar);
		}

		else
		{
			Ar << m_SourcePath; 
			MaterialParameters.Serialize(Ar);
		}
	}

#if WITH_EDITOR
	void MaterialInstance::Import()
	{
		AssetImporterMaterial::Import( this, m_SourcePath );
		Save();
		Load();

		MarkRenderStateDirty();
	}

	void MaterialInstance::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewMaterialInstanceGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void MaterialInstance::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}

#endif

}