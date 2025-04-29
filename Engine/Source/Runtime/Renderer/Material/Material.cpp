#include "DrnPCH.h"
#include "Material.h"
#include "Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"

namespace Drn
{
	Material::Material( const std::string& InPath )
		: Asset(InPath)
		, m_VS_Blob(nullptr)
		, m_PS_Blob(nullptr)
		, m_GS_Blob(nullptr)
		, m_HS_Blob(nullptr)
		, m_DS_Blob(nullptr)
		, m_CS_Blob(nullptr)
	{
		Load();
	}

#if WITH_EDITOR
	Material::Material( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_VS_Blob(nullptr)
		, m_PS_Blob(nullptr)
		, m_GS_Blob(nullptr)
		, m_HS_Blob(nullptr)
		, m_DS_Blob(nullptr)
		, m_CS_Blob(nullptr)
	{
		m_SourcePath = InSourcePath;
		Import();
	}
#endif
	
	Material::~Material()
	{
		ReleaseShaderBlobs();
	}

	EAssetType Material::GetAssetType() { return EAssetType::Material; }

	void Material::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			ReleaseShaderBlobs();

			Ar >> m_SourcePath;
			Ar >> m_VS_Blob >> m_PS_Blob >> m_GS_Blob >> m_HS_Blob >> m_DS_Blob >> m_CS_Blob;
		}

		else
		{
			Ar << m_SourcePath; 
			Ar << m_VS_Blob << m_PS_Blob << m_GS_Blob << m_HS_Blob << m_DS_Blob << m_CS_Blob;
		}
	}

#if WITH_EDITOR
	void Material::Import()
	{
		AssetImporterMaterial::Import( this, m_SourcePath );
		Save();
		Load();
	}

	void Material::ReleaseShaderBlobs()
	{
		if (m_VS_Blob) m_VS_Blob->Release();
		if (m_PS_Blob) m_PS_Blob->Release();
		if (m_GS_Blob) m_GS_Blob->Release();
		if (m_HS_Blob) m_HS_Blob->Release();
		if (m_DS_Blob) m_DS_Blob->Release();
		if (m_CS_Blob) m_CS_Blob->Release();
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
}