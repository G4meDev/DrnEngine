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
		: m_OwningAsset(InOwningAsset)
	{
		
	}

	void AssetPreviewStaticMeshGuiLayer::Draw()
	{
		if (!ImGui::Begin(m_OwningAsset->GetPath().c_str()))
		{
			ImGui::End();
			return;
		}

		ImGui::Text( "%s", m_OwningAsset->GetSourcePath().c_str());

		ImGui::ShowDemoWindow();

		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::SetCurrentFocus()
	{

	}
}

#endif