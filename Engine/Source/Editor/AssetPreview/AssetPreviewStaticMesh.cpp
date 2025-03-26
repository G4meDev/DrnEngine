#include "DrnPCH.h"
#include "AssetPreviewStaticMesh.h"

#if WITH_EDITOR

#include "Editor/Editor.h"
#include "AssetPreviewStaticMeshGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterStaticMesh.h"

namespace Drn
{
	// constructor for opening existing asset menu
	AssetPreviewStaticMesh::AssetPreviewStaticMesh(const std::string InPath)
		: AssetPreview(InPath)
	{
		std::unique_ptr<Archive> Ar = std::make_unique<Archive>( InPath, true );
		Serialize( *Ar.get() );

		GuiLayer = std::make_unique<AssetPreviewStaticMeshGuiLayer>(this);
		GuiLayer->Attach();
	}
	 
	// constructor for creating asset from source
	AssetPreviewStaticMesh::AssetPreviewStaticMesh(const std::string& InPath, const std::string& InSourcePath)
		: AssetPreview(InPath, InSourcePath)
	{
		Reimport();

		Save();
	}

	AssetPreviewStaticMesh::~AssetPreviewStaticMesh()
	{
		if (GuiLayer)
		{
			GuiLayer->DeAttach();
			GuiLayer.reset();
		}
	}

	void AssetPreviewStaticMesh::SetCurrentFocus()
	{
		
	}

	void AssetPreviewStaticMesh::Reimport()
	{
		AssetImporterStaticMesh::Import(this, m_SourcePath);
	}

	EAssetType AssetPreviewStaticMesh::GetAssetType()
	{
		return EAssetType::StaticMesh;
	}

	void AssetPreviewStaticMesh::Serialize( Archive& Ar )
	{
		AssetPreview::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> ImportScale;
		}
		
		else
		{
			Ar << ImportScale;
		}
	}

	void AssetPreviewStaticMesh::Save()
	{
		std::unique_ptr<Archive> Ar = std::make_unique<Archive>(m_Path, false);
		Serialize(*Ar.get());
	}

	uint8 AssetPreviewStaticMesh::AddMaterial( MaterialData Material )
	{
		for (int i = 0; i < MaterialsData.size(); i++)
		{
			if (MaterialsData[i].Name == Material.Name)
			{
				return i;
			}
		}

		MaterialsData.push_back(Material);
		return MaterialsData.size() - 1;
	}

}

#endif