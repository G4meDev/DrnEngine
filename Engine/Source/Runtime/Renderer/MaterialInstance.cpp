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
	}

#if WITH_EDITOR
	MaterialInstance::MaterialInstance( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
	{
		m_SourcePath = InSourcePath;
		Import();
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
		
	}

	void MaterialInstance::BindResources( D3D12CommandList* CommandList )
	{
		
	}

	void MaterialInstance::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		
	}

	void MaterialInstance::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		
	}

	void MaterialInstance::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		
	}

	void MaterialInstance::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		
	}

	void MaterialInstance::SetIndexedScalar( uint32 Index, float Value )
	{
		
	}

	void MaterialInstance::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		
	}

	void MaterialInstance::SetNamedScalar( const std::string& Name, float Value )
	{
		
	}

	void MaterialInstance::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		
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