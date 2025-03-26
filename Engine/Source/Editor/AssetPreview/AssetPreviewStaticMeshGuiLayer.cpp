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

		DrawSidePanel();

		ImGui::ShowDemoWindow();

		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::DrawSidePanel()
	{
		ImGui::Begin("Details");

		if (ImGui::Button( "save" ))
		{
			m_OwningAsset->Save();
		}

		ImGui::Separator();

		ImGui::Text("source file: %s", m_OwningAsset->GetSourcePath() != NAME_NULL ? m_OwningAsset->GetSourcePath().c_str() : "...");
		
		if (ImGui::Button("reimport"))
		{
			m_OwningAsset->Reimport();
		}

		ImGui::Button("select");

		ImGui::Separator();

		ImGui::InputFloat( "ImportScale", &m_OwningAsset->ImportScale);
		
		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::SetCurrentFocus()
	{
		
	}

}

#endif