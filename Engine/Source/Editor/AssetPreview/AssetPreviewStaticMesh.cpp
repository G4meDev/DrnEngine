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
			MeshData.Serialize(Ar);
			Ar >> ImportScale;
		}
		
		else
		{
			MeshData.Serialize(Ar);
			Ar << ImportScale;
		}
	}

	void AssetPreviewStaticMesh::Save()
	{
		std::unique_ptr<Archive> Ar = std::make_unique<Archive>(m_Path, false);
		Serialize(*Ar.get());
	}

}

#endif