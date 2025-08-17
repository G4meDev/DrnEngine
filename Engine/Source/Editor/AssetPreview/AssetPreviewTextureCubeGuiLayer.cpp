#include "DrnPCH.h"
#include "AssetPreviewTextureCubeGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/Texture/TextureHelper.h"
#include "Editor/Editor.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include <imgui.h>

namespace Drn
{
	AssetPreviewTextureCubeGuiLayer::AssetPreviewTextureCubeGuiLayer( TextureCube* InOwningAsset )
	{
		m_OwningAsset = AssetHandle<TextureCube>( InOwningAsset->m_Path );
		m_OwningAsset.Load();

		m_PreviewWorld = WorldManager::Get()->AllocateWorld();

		AssetHandle<StaticMesh> PlaneMesh( "Engine\\Content\\BasicShapes\\SM_Quad.drn" );
		PlaneMesh.Load();
		
		AssetHandle<Material> TextureCubePreviewMaterial( "Engine\\Content\\Materials\\M_TextureCubePreview.drn" );
		TextureCubePreviewMaterial.Load();
		TextureCubePreviewMaterial->SetNamedTextureCube("Texture", m_OwningAsset);

		m_PreviewMeshPlane = m_PreviewWorld->SpawnActor<StaticMeshActor>();
		m_PreviewMeshPlane->GetMeshComponent()->SetSelectable( false );
		m_PreviewMeshPlane->GetMeshComponent()->SetEditorPrimitive( true);
		m_PreviewMeshPlane->GetMeshComponent()->SetMesh( PlaneMesh );
		m_PreviewMeshPlane->GetMeshComponent()->SetMaterial(0, TextureCubePreviewMaterial);

// ------------------------------------------------------------------------------------------------------

		AssetHandle<StaticMesh> SphereMesh( "Engine\\Content\\BasicShapes\\SM_Sphere.drn" );
		SphereMesh.Load();
		
		AssetHandle<Material> TextureCubePreview3DMaterial( "Engine\\Content\\Materials\\M_TextureCube3DPreview.drn" );
		TextureCubePreview3DMaterial.Load();
		TextureCubePreview3DMaterial->SetNamedTextureCube("Texture", m_OwningAsset);

		StaticMeshActor* m_PreviewMeshSphere = m_PreviewWorld->SpawnActor<StaticMeshActor>();
		m_PreviewMeshSphere->GetMeshComponent()->SetSelectable( false );
		m_PreviewMeshSphere->GetMeshComponent()->SetEditorPrimitive( true);
		m_PreviewMeshSphere->GetMeshComponent()->SetMesh( SphereMesh );
		m_PreviewMeshSphere->GetMeshComponent()->SetMaterial(0, TextureCubePreview3DMaterial);
		m_PreviewMeshSphere->SetActorLocation(Vector(0, 4, 0));

// ------------------------------------------------------------------------------------------------------

		m_ViewportPanel = std::make_unique<ViewportPanel>( m_PreviewWorld->GetScene() );
	}

	AssetPreviewTextureCubeGuiLayer::~AssetPreviewTextureCubeGuiLayer()
	{
		if (m_PreviewWorld)
		{
			m_PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewTextureCubeGuiLayer::Draw( float DeltaTime )
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

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize), 0.0f );

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			m_ViewportPanel->SetRenderingEnabled(true);
			m_ViewportPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			DrawDetailsPanel();

			ImGui::EndChild();
		}

		ImGui::End();
	}

	void AssetPreviewTextureCubeGuiLayer::DrawMenu()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
			if ( ImGui::BeginMenu( "Debug" ) )
			{
				if ( ImGui::MenuItem( "log live assets" ) )
				{
					AssetManager::Get()->ReportLiveAssets();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void AssetPreviewTextureCubeGuiLayer::DrawDetailsPanel()
	{
		ImGui::Text("Detail Panel");

		if ( ImGui::Button( "Reimport" ) )
		{
			m_OwningAsset->Import();
		}
		 
		ImGui::Checkbox( "sRGB", &m_OwningAsset->m_sRGB );

		ImGui::Separator();

		ImGui::Text( "%s", TextureHelper::GetDxgiFormatName(m_OwningAsset->GetFormat()).c_str() );
		ImGui::Text( "%ux%ux6", m_OwningAsset->GetSizeX(), m_OwningAsset->GetSizeY() );
		ImGui::Text( "%u mips", m_OwningAsset->GetMipLevels() );
	}
}

#endif