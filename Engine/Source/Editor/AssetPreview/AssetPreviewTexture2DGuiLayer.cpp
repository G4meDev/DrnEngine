#include "DrnPCH.h"
#include "AssetPreviewTexture2DGuiLayer.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include "Runtime/Renderer/Texture/TextureHelper.h"

namespace Drn
{
	AssetPreviewTexture2DGuiLayer::AssetPreviewTexture2DGuiLayer( Texture2D* InOwningAsset )
		: m_MipLevel(0)
	{
		m_OwningAsset = AssetHandle<Texture2D>( InOwningAsset->m_Path );
		m_OwningAsset.Load();

		m_PreviewWorld = WorldManager::Get()->AllocateWorld();
		m_PreviewWorld->SetTransient(true);
		m_PreviewWorld->SetPaused(true);
		m_PreviewWorld->SetEditorWorld();

		AssetHandle<StaticMesh> PlaneMesh( "Engine\\Content\\BasicShapes\\SM_Quad.drn" );
		PlaneMesh.Load();
		
		m_PreviewMaterial = AssetHandle<Material>( "Engine\\Content\\Materials\\M_Texture2DPreview.drn" );
		m_PreviewMaterial.Load();
		m_PreviewMaterial->SetNamedTexture2D("Texture", m_OwningAsset);

		m_PreviewMeshPlane = m_PreviewWorld->SpawnActor<StaticMeshActor>();
		m_PreviewMeshPlane->GetMeshComponent()->SetSelectable(false);
		m_PreviewMeshPlane->GetMeshComponent()->SetMesh( PlaneMesh );
		m_PreviewMeshPlane->GetMeshComponent()->SetMaterial(0, m_PreviewMaterial);
		m_PreviewMeshPlane->SetActorLocation(Vector(0, 4, 0));

		m_ViewportPanel = std::make_unique<ViewportPanel>( m_PreviewWorld->GetScene() );
		UpdateMipLevel();
	}

	AssetPreviewTexture2DGuiLayer::~AssetPreviewTexture2DGuiLayer()
	{
		if (m_PreviewWorld)
		{
			m_PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}


	void AssetPreviewTexture2DGuiLayer::Draw( float DeltaTime )
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

	void AssetPreviewTexture2DGuiLayer::DrawMenu()
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

	void AssetPreviewTexture2DGuiLayer::DrawDetailsPanel()
	{
		ImGui::Text("Detail Panel");

		if ( ImGui::Button( "Reimport" ) )
		{
			m_OwningAsset->Import();
		}
		 
		ImGui::Checkbox( "sRGB", &m_OwningAsset->m_sRGB );

		ImGui::Checkbox( "Generate Mips", &m_OwningAsset->m_GenerateMips );

		ImGui::Separator();

		ImGui::Text( "%s", TextureHelper::GetDxgiFormatName(m_OwningAsset->GetFormat()).c_str() );
		ImGui::Text( "%ux%u", m_OwningAsset->GetSizeX(), m_OwningAsset->GetSizeY() );
		ImGui::Text( "%u mips", m_OwningAsset->GetMipLevels() );

		const char* LayoutTypes[] = { "NoCompression", "BC1", "BC4", "BC5", "BC6", "BC2" };
		int32 CurrrentComppression = static_cast<int32>(m_OwningAsset->m_Compression);
		if ( ImGui::Combo( "Compression", &CurrrentComppression, LayoutTypes, IM_ARRAYSIZE( LayoutTypes )))
		{
			m_OwningAsset->m_Compression = static_cast<ETextureCompression>(CurrrentComppression);
		}

		ImGui::Separator();

		if ( ImGui::DragFloat( "Mip Level", &m_MipLevel, 0.05f, 0, m_OwningAsset->m_MipLevels) )
		{
			UpdateMipLevel();
		}

		bool ShowColorDirty = false;
		ShowColorDirty |= ImGui::Checkbox("R", &m_ShowR);
		ShowColorDirty |= ImGui::Checkbox("G", &m_ShowG);
		ShowColorDirty |= ImGui::Checkbox("B", &m_ShowB);
		ShowColorDirty |= ImGui::Checkbox("A", &m_ShowA);

		if (ShowColorDirty)
		{
			UpdateShowColor();
		}
	}

	void AssetPreviewTexture2DGuiLayer::UpdateMipLevel()
	{
		m_PreviewMaterial->SetNamedScalar("MipLevel", m_MipLevel);
	}

	void AssetPreviewTexture2DGuiLayer::UpdateShowColor()
	{
		m_PreviewMaterial->SetNamedVector4("ShowColor", Vector4(m_ShowR, m_ShowG, m_ShowB, m_ShowA));
	}

}

#endif