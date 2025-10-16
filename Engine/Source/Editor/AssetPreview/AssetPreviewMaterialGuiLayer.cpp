#include "DrnPCH.h"
#include "AssetPreviewMaterialGuiLayer.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/Editor.h"

LOG_DEFINE_CATEGORY( LogAssetPreviewMaterial, "AssetPreviewMaterial" );

namespace Drn
{
	AssetPreviewMaterialGuiLayer::AssetPreviewMaterialGuiLayer( Material* InOwningAsset )
		: m_ShowDetailsPanel(true)
		, m_ShowSceneSettingsPanel(true)
	{
		LOG( LogAssetPreviewMaterial, Info, "opening %s", InOwningAsset->m_Path.c_str() );

		m_OwningAsset = AssetHandle<Material>( InOwningAsset->m_Path );
		m_OwningAsset.Load();

		PreviewWorld = WorldManager::Get()->AllocateWorld();
		PreviewWorld->SetTransient(true);
		PreviewWorld->SetPaused(true);
		PreviewWorld->SetEditorWorld();

		AssetHandle<StaticMesh> SphereMesh( "Engine\\Content\\BasicShapes\\SM_Sphere.drn" );
		SphereMesh.Load();
		
		PreviewMesh = PreviewWorld->SpawnActor<StaticMeshActor>();
		PreviewMesh->GetMeshComponent()->SetSelectable( false );
		PreviewMesh->GetMeshComponent()->SetMesh( SphereMesh );
		PreviewMesh->GetMeshComponent()->SetMaterial(0, m_OwningAsset);

		m_SkyLight = PreviewWorld->SpawnActor<SkyLightActor>();
		m_SkyLight->SetIntensity(0.4f);

		m_DirectionalLight = PreviewWorld->SpawnActor<DirectionalLightActor>();
		m_DirectionalLight->SetIntensity(1);
		m_DirectionalLight->SetActorRotation(Quat(0, XM_PIDIV4, XM_PI));

		m_ViewportPanel = std::make_unique<ViewportPanel>( PreviewWorld->GetScene() );
	}

	AssetPreviewMaterialGuiLayer::~AssetPreviewMaterialGuiLayer()
	{
		LOG(LogAssetPreviewMaterial, Info, "closing %s", m_OwningAsset->m_Path.c_str());

		if (PreviewWorld)
		{
			PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewMaterialGuiLayer::Draw( float DeltaTime )
	{
		SCOPE_STAT();

		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
			m_ViewportPanel->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

		DrawMenu();

		ImVec2 Size = ImGui::GetContentRegionAvail();
		float BorderSize = ImGui::GetStyle().FramePadding.x;

		bool bLeftPanel = m_ShowSceneSettingsPanel;
		bool bRightPanel = m_ShowDetailsPanel;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize) * (bLeftPanel + bRightPanel) , 0.0f );

		if ( m_ShowSceneSettingsPanel && ImGui::BeginChild( "Scene Setting", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{

		}
		ImGui::EndChild();

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			m_ViewportPanel->SetRenderingEnabled(true);
			m_ViewportPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (m_ShowDetailsPanel && ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			DrawDetailsPanel();

		}
		ImGui::EndChild();


		ImGui::End();
	}

	void AssetPreviewMaterialGuiLayer::DrawMenu()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
			if (ImGui::BeginMenu( "Window" ))
			{
				ImGui::MenuItem( "Scene Setting", NULL, &m_ShowSceneSettingsPanel);
				ImGui::MenuItem( "Detail", NULL, &m_ShowDetailsPanel);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void AssetPreviewMaterialGuiLayer::DrawDetailsPanel()
	{
		if (ImGui::Button("Import"))
		{
			m_OwningAsset->Import();

#if WITH_EDITOR
			Editor::Get()->NotifyMaterialReimported(m_OwningAsset);
#endif
		}

		const char* PrimitiveTypes[] = { "Point", "Line", "Triangle", "Patch" };
		int CurrentType = static_cast<int>(m_OwningAsset->m_PrimitiveType) - 1;
		if ( ImGui::Combo( "Primitive Type", &CurrentType, PrimitiveTypes, IM_ARRAYSIZE( PrimitiveTypes )))
		{
			m_OwningAsset->m_PrimitiveType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(CurrentType + 1);
		}

		const char* LayoutTypes[] = { "StandardMesh", "LineColorThickness", "Position", "StandardMeshTemp" };
		CurrentType = static_cast<int>(m_OwningAsset->m_InputLayoutType);
		if ( ImGui::Combo( "Layout Type", &CurrentType, LayoutTypes, IM_ARRAYSIZE( LayoutTypes )))
		{
			m_OwningAsset->m_InputLayoutType = static_cast<EInputLayoutType>(CurrentType);
		}

		ImGui::Checkbox( "TwoSided", &m_OwningAsset->m_TwoSided );

		DrawParameters();
	}

	void AssetPreviewMaterialGuiLayer::DrawParameters()
	{
		ImGui::Separator();

		if (ImGui::CollapsingHeader( "Texture2D", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen) )
		{
			for (int i = 0; i < m_OwningAsset->m_Texture2DSlots.size(); i++)
			{
				AssetHandle<Texture2D> DropedTexture = m_OwningAsset->m_Texture2DSlots[i].Draw();

				if (DropedTexture.IsValid())
				{
					m_OwningAsset->SetIndexedTexture2D(i, DropedTexture);
				}
			}
		}

		if (ImGui::CollapsingHeader( "TextureCube", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen) )
		{
			for (int i = 0; i < m_OwningAsset->m_TextureCubeSlots.size(); i++)
			{
				AssetHandle<TextureCube> DropedTexture = m_OwningAsset->m_TextureCubeSlots[i].Draw();

				if (DropedTexture.IsValid())
				{
					m_OwningAsset->SetIndexedTextureCube(i, DropedTexture);
				}
			}
		}

		if (ImGui::CollapsingHeader( "Scalar", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen) )
		{
			for (int i = 0; i < m_OwningAsset->m_FloatSlots.size(); i++)
			{
				if (m_OwningAsset->m_FloatSlots[i].Draw())
				{
					m_OwningAsset->SetNamedScalar(m_OwningAsset->m_FloatSlots[i].m_Name,
						m_OwningAsset->m_FloatSlots[i].m_Value);
				}
			}
		}

		if (ImGui::CollapsingHeader( "Vector4", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen) )
		{
			for (int i = 0; i < m_OwningAsset->m_Vector4Slots.size(); i++)
			{
				if (m_OwningAsset->m_Vector4Slots[i].Draw())
				{
					m_OwningAsset->SetNamedVector4(m_OwningAsset->m_Vector4Slots[i].m_Name,
						m_OwningAsset->m_Vector4Slots[i].m_Value);
				}
			}
		}
	}

}

#endif