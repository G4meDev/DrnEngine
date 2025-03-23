#include "DrnPCH.h"
#include "AssetPreviewStaticMesh.h"

#if WITH_EDITOR

#include "Editor/Editor.h"
#include "AssetPreviewStaticMeshGuiLayer.h"

namespace Drn
{
	AssetPreviewStaticMesh::AssetPreviewStaticMesh(const std::string InPath)
		: AssetPreview(InPath)
	{
		GuiLayer = std::make_unique<AssetPreviewStaticMeshGuiLayer>(this);
		GuiLayer->Attach();
	}

	AssetPreviewStaticMesh::~AssetPreviewStaticMesh()
	{
		GuiLayer->DeAttach();
		GuiLayer.reset();
	}

	void AssetPreviewStaticMesh::SetCurrentFocus()
	{
		
	}
}

#endif