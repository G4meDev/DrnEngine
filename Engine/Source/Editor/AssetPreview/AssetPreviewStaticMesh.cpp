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
		GuiLayer = std::make_unique<AssetPreviewStaticMeshGuiLayer>(this);
		GuiLayer->Attach();
	}
	 
	// constructor for creating asset from source
	AssetPreviewStaticMesh::AssetPreviewStaticMesh(const std::string& InPath, const std::string& InSourcePath)
		: AssetPreview(InPath, InSourcePath)
	{
		std::ofstream outFile( InPath, std::ios::binary );
		if (!outFile)
		{
			LOG( LogAssetPreview, Error, "failed to create asset file. " );
			return;
		}

		uint8 type = static_cast<uint8>(GetAssetType());
		uint8 SourceFileSize = InSourcePath.size();
		
		//outFile<<(reinterpret_cast<const char*>(&type), sizeof(type));
		//outFile<<(reinterpret_cast<const char*>(&SourceFileSize), sizeof(SourceFileSize));
		//outFile.write(InSourcePath.c_str(), SourceFileSize);
		outFile<<type;
		outFile<<SourceFileSize;
		outFile<<InSourcePath.c_str();
		outFile.close();
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

}

#endif