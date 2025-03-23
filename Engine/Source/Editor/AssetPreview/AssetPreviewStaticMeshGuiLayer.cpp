#include "DrnPCH.h"
#include "AssetPreviewStaticMeshGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "AssetPreviewStaticMesh.h"

namespace Drn
{
	AssetPreviewStaticMeshGuiLayer::AssetPreviewStaticMeshGuiLayer(AssetPreviewStaticMesh* InOwningAsset)
		: OwningAsset(InOwningAsset)
	{
		
	}

	void AssetPreviewStaticMeshGuiLayer::Draw()
	{
		if (!ImGui::Begin(OwningAsset->GetPath().c_str()))
		{
			ImGui::End();
			return;
		}



		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::SetCurrentFocus()
	{

	}
}

#endif