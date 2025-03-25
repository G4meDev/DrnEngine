#include "DrnPCH.h"
#include "AssetPreviewStaticMesh.h"

#if WITH_EDITOR

#include "Editor/Editor.h"
#include "AssetPreviewStaticMeshGuiLayer.h"

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
		std::unique_ptr<Archive> Ar = std::make_unique<Archive>(InPath, false);
		Serialize(*Ar.get());
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
		
	}

	EAssetType AssetPreviewStaticMesh::GetAssetType()
	{
		return EAssetType::StaticMesh;
	}

	void AssetPreviewStaticMesh::Serialize( Archive& Ar )
	{
		AssetPreview::Serialize(Ar);
	}
}

#endif