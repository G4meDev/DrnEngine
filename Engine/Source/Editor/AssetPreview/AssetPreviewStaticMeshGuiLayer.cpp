#include "DrnPCH.h"
#include "AssetPreviewStaticMeshGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"
#include "Editor/FileImportMenu/FileImportMenu.h"
#include "Editor/EditorPanels/ViewportPanel.h"

LOG_DEFINE_CATEGORY( LogStaticMeshPreview, "StaticMeshPreview" );

namespace Drn
{
	AssetPreviewStaticMeshGuiLayer::AssetPreviewStaticMeshGuiLayer(StaticMesh* InOwningAsset)
		: m_ShowSceneSetting(true)
		, m_ShowDetail(true)
		, m_DebugLinesSize(0.1f)
		, m_DrawNormals(false)
		, m_DrawTangents(false)
		, m_DrawBitTangents(false)
	{
		LOG(LogStaticMeshPreview, Info, "opening %s", InOwningAsset->m_Path.c_str());

		m_OwningAsset = AssetHandle<StaticMesh>(InOwningAsset->m_Path);
		m_OwningAsset.Load();

		PreviewWorld = WorldManager::Get()->AllocateWorld();
		PreviewWorld->SetTransient(true);
		PreviewWorld->SetPaused(true);
		PreviewWorld->SetEditorWorld();

		PreviewMesh = PreviewWorld->SpawnActor<StaticMeshActor>();
		PreviewMesh->GetMeshComponent()->SetSelectable(false);
		PreviewMesh->GetMeshComponent()->SetMesh(m_OwningAsset);

		m_SkyLight = PreviewWorld->SpawnActor<SkyLightActor>();
		m_SkyLight->SetIntensity(0.4f);

		m_DirectionalLight = PreviewWorld->SpawnActor<DirectionalLightActor>();
		m_DirectionalLight->SetIntensity(1);
		m_DirectionalLight->SetActorRotation(Quat(0, XM_PIDIV4, XM_PI));

		m_ViewportPanel = std::make_unique<ViewportPanel>(PreviewWorld->GetScene());
	}

	AssetPreviewStaticMeshGuiLayer::~AssetPreviewStaticMeshGuiLayer()
	{
		LOG(LogStaticMeshPreview, Info, "closing %s", m_OwningAsset->m_Path.c_str());

		if (PreviewWorld)
		{
			PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewStaticMeshGuiLayer::Draw( float DeltaTime )
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

		DrawDebugs();

		DrawMenu();

		ImVec2 Size = ImGui::GetContentRegionAvail();
		float BorderSize = ImGui::GetStyle().FramePadding.x;

		bool bLeftPanel = m_ShowSceneSetting;
		bool bRightPanel = m_ShowDetail;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize) * (bLeftPanel + bRightPanel) , 0.0f );

		if ( m_ShowSceneSetting && ImGui::BeginChild( "Scene Setting", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
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
		if (m_ShowDetail && ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			DrawDetailPanel();

		}
		ImGui::EndChild();


		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::DrawMenu()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
			if ( ImGui::BeginMenu( "File" ) )
			{
				ImGui::MenuItem( "nothing" );
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Window" ))
			{
				ImGui::MenuItem( "Scene Setting", NULL, &m_ShowSceneSetting);
				ImGui::MenuItem( "Detail", NULL, &m_ShowDetail);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "View" ))
			{
				auto DrawViewFlag = [&](EWorldViewFlag Flag, const char* Name)
				{
					bool ViewHasFlag = PreviewWorld->HasViewFlag(Flag);
					if (ImGui::MenuItem( Name, NULL, &ViewHasFlag))
					{
						PreviewWorld->SetViewFlag(Flag, ViewHasFlag);
					}
				};

				DrawViewFlag( EWorldViewFlag::Collision, "Collision" );

				ImGui::EndMenu();
			}

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

	void AssetPreviewStaticMeshGuiLayer::DrawDetailPanel()
	{
		if (ImGui::Button( "save" ))
		{
			m_OwningAsset.Get()->Save();
		}

		ImGui::Separator();

		ImGui::TextWrapped("source file: %s", m_OwningAsset.Get()->m_SourcePath != NAME_NULL ? m_OwningAsset.Get()->m_SourcePath.c_str() : "...");
		
		if (ImGui::Button("reimport"))
		{
			m_OwningAsset.Get()->Import();
		}

		if ( ImGui::Button( "select" ) )
		{
			ShowSourceFileSelection();
		}

		ImGui::Separator();

		uint64 VertexCount = 0;
		for (auto& MeshSlot : m_OwningAsset->Data.MeshesData)
		{
			VertexCount += MeshSlot.m_VertexCount;
		}

		ImGui::Text( "%u vertecies", VertexCount);

		ImGui::Separator();

		ImGui::Separator();
		ImGui::Text("Materials");
		
		for (MaterialData& Mat : m_OwningAsset->Data.Materials)
		{
			ImGui::Text(Mat.m_Name.c_str());
			ImGui::Text(Mat.m_Material.GetPath().c_str());

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
				{
					auto AssetPath = static_cast<const char*>(payload->Data);
					AssetHandle<Asset> DropedMaterial(AssetPath);
					EAssetType Type = DropedMaterial.LoadGeneric();
					
					if (Type == EAssetType::Material)
					{
						Mat.m_Material = AssetHandle<Material>(AssetPath);
						PreviewMesh->GetMeshComponent()->MarkRenderStateDirty();
					}
				}

				ImGui::EndDragDropTarget();
			}
		}

		ImGui::Separator();
		ImGui::InputFloat( "ImportScale", &m_OwningAsset.Get()->ImportScale);
		ImGui::Separator();

		m_OwningAsset->m_BodySetup.DrawDetailPanel();

// ------------------------------------------------------------------------------------------------------

		ImGui::Checkbox( "Use Complex Collision", &m_OwningAsset->m_BodySetup.m_UseTriMesh );
		ImGui::Separator();

// ------------------------------------------------------------------------------------------------------

		ImGui::Checkbox( "Import Normal", &m_OwningAsset->m_ImportNormals);
		ImGui::Checkbox( "Import Tangent", &m_OwningAsset->m_ImportTangents);
		ImGui::Checkbox( "Import BitTangent", &m_OwningAsset->m_ImportBitTangents);
		ImGui::Checkbox( "Import Color", &m_OwningAsset->m_ImportColor);
		int32 UvNum = m_OwningAsset->m_ImportUVs;
		ImGui::SliderInt( "Import UVs", &UvNum, 0, 8);
		m_OwningAsset->m_ImportUVs = UvNum;

		ImGui::Separator();

// ------------------------------------------------------------------------------------------------------

		ImGui::Checkbox( "Draw Normals", &m_DrawNormals);
		ImGui::Checkbox( "Draw Tangent", &m_DrawTangents);
		ImGui::Checkbox( "Draw BitTangents", &m_DrawBitTangents);
		ImGui::DragFloat( "Line Size", &m_DebugLinesSize, 0.001, 10);
	}

	void AssetPreviewStaticMeshGuiLayer::ShowSourceFileSelection()
	{
		Editor::Get()->OpenImportMenu(
			"Select source file", FileImportMenu::FileFilter_Any(),
			std::bind( &AssetPreviewStaticMeshGuiLayer::OnSelectedSourceFile, this, std::placeholders::_1 ) );
	}

	void AssetPreviewStaticMeshGuiLayer::OnSelectedSourceFile( std::string FilePath )
	{
		m_OwningAsset->m_SourcePath = FilePath;
		m_OwningAsset->Import();
	}

	void AssetPreviewStaticMeshGuiLayer::DrawDebugs()
	{
		if (m_DrawNormals || m_DrawTangents || m_DrawBitTangents)
		{
			for ( StaticMeshSlotData& Data : m_OwningAsset->Data.MeshesData )
			{
				for (uint64 i = 0; i < Data.Positions.size(); i++)
				{
					const Vector& Pos = Data.Positions[i];

					if (m_DrawNormals && Data.VertexData.HasNormals())
					{
						const Vector& Normal = Data.VertexData.GetNormals()[i];
						PreviewWorld->DrawDebugLine( Pos, Pos + Normal * m_DebugLinesSize, Color::Green, 0, 0 );
					}

					if (m_DrawTangents && Data.VertexData.HasTangents())
					{
						const Vector& Tangent = Data.VertexData.GetTangents()[i];
						PreviewWorld->DrawDebugLine( Pos, Pos + Tangent * m_DebugLinesSize, Color::Blue, 0, 0 );
					}

					if (m_DrawBitTangents && Data.VertexData.HasBitTangents())
					{
						const Vector& BitTangent = Data.VertexData.GetBitTangents()[i];
						PreviewWorld->DrawDebugLine( Pos, Pos + BitTangent * m_DebugLinesSize, Color::Red, 0, 0 );
					}
				}
			}
		}
	}

	void AssetPreviewStaticMeshGuiLayer::SetCurrentFocus()
	{
		
	}

}

#endif