#include "DrnPCH.h"
#include "LevelViewportGuiLayer.h"

#if WITH_EDITOR

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"
#include "Editor/Misc/EditorMisc.h"

#include <imgui.h>

namespace Drn
{
	LevelViewportGuiLayer::LevelViewportGuiLayer(LevelViewport* InOwningLevelViewport)
		: m_OwningLevelViewport(InOwningLevelViewport)
		, m_ShowModes(true)
		, m_ShowOutliner(true)
		, m_ShowDetail(true)
	{
		m_ViewportPanel = std::make_unique<ViewportPanel>( WorldManager::Get()->GetMainWorld()->GetScene() );
		m_ViewportPanel->OnSelectedNewComponent.Add( InOwningLevelViewport, &LevelViewport::OnSelectedNewComponent );
		m_ViewportPanel->GetSelectedComponentDel.Bind( InOwningLevelViewport, &LevelViewport::GetSelectedComponent );
		m_ViewportPanel->HandleInputDel.Bind( this, &LevelViewportGuiLayer::HandleViewportInputs );
		
		m_WorldOutlinerPanel = std::make_unique<WorldOutlinerPanel>(WorldManager::Get()->GetMainWorld() );
		m_WorldOutlinerPanel->OnSelectedNewComponent.Add( InOwningLevelViewport, &LevelViewport::OnSelectedNewComponent );
		m_WorldOutlinerPanel->GetSelectedComponentDel.Bind( InOwningLevelViewport, &LevelViewport::GetSelectedComponent);

		m_ActorDetailPanel = std::make_unique<ActorDetailPanel>();
		m_ActorDetailPanel->OnSelectedNewComponent.Add( InOwningLevelViewport, &LevelViewport::OnSelectedNewComponent );
		m_ActorDetailPanel->GetSelectedComponentDel.Bind( InOwningLevelViewport, &LevelViewport::GetSelectedComponent);

		m_ModesPanel = std::make_unique<ModesPanel>();
	}

	LevelViewportGuiLayer::~LevelViewportGuiLayer()
	{
		m_ViewportPanel->OnSelectedNewComponent.Remove( this );
	}

	void LevelViewportGuiLayer::Draw( float DeltaTime )
	{
		SCOPE_STAT();

		if (!ImGui::Begin(WorldManager::Get()->GetMainWorld()->m_WorldLabel.c_str()))
		{
			m_ViewportPanel->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

		DrawMenuBar(DeltaTime);

		ImVec2 Size = ImGui::GetContentRegionAvail();
		float BorderSize = ImGui::GetStyle().FramePadding.x;

		bool bLeftPanel = m_ShowOutliner || m_ShowModes;
		bool bRightPanel = m_ShowDetail;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize) * (bLeftPanel + bRightPanel) , 0.0f );

		if (bLeftPanel && ImGui::BeginChild("Mode", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if ( ImGui::BeginTabBar( "Tab" ))
			{
				if (m_ShowModes && ImGui::BeginTabItem(" Modes "))
				{
					m_ModesPanel->Draw(DeltaTime);
					ImGui::EndTabItem();
				}

				if (m_ShowOutliner && ImGui::BeginTabItem("Outliner"))
				{
					m_WorldOutlinerPanel->Draw(DeltaTime);
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::EndChild();
		}

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			DrawViewportMenu(DeltaTime);

			m_ViewportPanel->SetRenderingEnabled(true);
			m_ViewportPanel->Draw(DeltaTime);

			if ( ImGui::BeginDragDropTarget() )
			{
				if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
				{
					HandleViewportPayload(Payload);
				}

				else if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_EditorSpawnable()))
				{
					HandleEditorSpwan(Payload);
				}

				ImGui::EndDragDropTarget();
			}

			DrawContextPopup();

			ImGui::EndChild();
		}

		ImGui::SameLine();
		if (m_ShowDetail && ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			m_ActorDetailPanel->Draw(DeltaTime);
		
			ImGui::EndChild();
		}

		ImGui::End();
	}

	void LevelViewportGuiLayer::DrawMenuBar( float DeltaTime )
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu( "File" ))
			{
				if (ImGui::MenuItem( "Save level" ))
				{
					WorldManager::Get()->GetMainWorld()->Save();
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Window" ))
			{
				ImGui::MenuItem( "Modes", NULL, &m_ShowModes);
				ImGui::MenuItem( "Outliner", NULL, &m_ShowOutliner);
				ImGui::MenuItem( "Detail", NULL, &m_ShowDetail);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "View" ))
			{
				ImGui::MenuItem( "Collision", NULL, &WorldManager::Get()->GetMainWorld()->GetPhysicScene()->m_DrawDebugCollision);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Debug" ))
			{
				if ( ImGui::MenuItem( "log live assets" ) )
				{
					AssetManager::Get()->ReportLiveAssets();
				}

				if ( ImGui::MenuItem( "log live render objects" ) )
				{
					Renderer::Get()->ReportLiveObjects();
				}

				if ( ImGui::MenuItem( "task graph visualizer" ) )
				{
					Editor::Get()->OpenTaskGraphVisualizer();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Tools" ))
			{
				if (ImGui::MenuItem( "Optick"))
				{
					ShellExecute(NULL, "open", "..\\..\\..\\Tools\\Optick\\Optick.exe", NULL, NULL, SW_SHOWDEFAULT);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void LevelViewportGuiLayer::DrawViewportMenu( float DeltaTime )
	{
		if ( World* MainWorld = WorldManager::Get()->GetMainWorld() )
		{
			if (MainWorld->IsPlayInEditorWorld())
			{
				if (ImGui::Button("End Play"))
				{
					WorldManager::Get()->EndPlayInEditor();
				}

				ImGui::SameLine();
				if (ImGui::Button(MainWorld->IsPaused() ? "Unpause" : "Pause"))
				{
					MainWorld->SetPaused(!MainWorld->IsPaused());
				}

				ImGui::SameLine();
				if (ImGui::Button(MainWorld->IsEjected() ? "Possess" : "Eject"))
				{
					MainWorld->SetEjected(!MainWorld->IsEjected());
				}
			}
			
			else
			{
				if (ImGui::Button("Play"))
				{
					WorldManager::Get()->StartPlayInEditor();
				}
			}
		}
	}

	void LevelViewportGuiLayer::HandleViewportPayload( const ImGuiPayload* Payload )
	{
		auto AssetPath = static_cast<const char*>(Payload->Data);

		AssetHandle<Asset> asset(AssetPath);
		EAssetType Type = asset.LoadGeneric();

		if (asset.IsValid() && Type == EAssetType::StaticMesh)
		{
			StaticMeshActor* NewActor = WorldManager::Get()->GetMainWorld()->SpawnActor<StaticMeshActor>();

			AssetHandle<StaticMesh> MeshAsset(AssetPath);
			MeshAsset.Load();
			NewActor->GetMeshComponent()->SetMesh(MeshAsset);
		}
	}

	void LevelViewportGuiLayer::HandleViewportInputs()
	{
		if (ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Right ))
		{
			ImGui::OpenPopup( "ContextMenuPopup" );
		}

		if ( ImGui::IsKeyPressed(ImGuiKey_Delete) )
		{
			DeleteSelectedActor();
		}

		if ( ImGui::IsKeyPressed(ImGuiKey_C) )
		{
			AlignSelectedComponentToSurfaceBelow();
		}

	}

	void LevelViewportGuiLayer::HandleEditorSpwan( const ImGuiPayload* Payload )
	{
		if (EditorLevelSpawnable** Spawnable = reinterpret_cast<EditorLevelSpawnable**>(Payload->Data))
		{
			Actor* SpawnActor = (*Spawnable)->SpawnFunc(m_OwningLevelViewport->m_OwningWorld);
		}
	}

	void LevelViewportGuiLayer::OnHitPlay()
	{
		
	}

	void LevelViewportGuiLayer::DrawContextPopup()
	{
		if ( ImGui::BeginPopup( "ContextMenuPopup" ) )
		{
			if (ImGui::Button("Edit"))
			{
				ImGui::OpenPopup( "EditPopup" );
			}

			if (ImGui::BeginPopup( "EditPopup" ))
			{
				if (ImGui::Button( "Delete" ))
				{
					DeleteSelectedActor();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Button( "unused_1" );
				ImGui::Button( "unused_2" );
				ImGui::Button( "unused_3" );

				ImGui::Separator();

				std::string SelectedActorName = m_OwningLevelViewport->GetSelectedComponent() ?
					m_OwningLevelViewport->GetSelectedComponent()->GetOwningActor()->GetActorLabel() : "Nothing Selected";

				ImGui::Text( SelectedActorName.c_str() );

				ImGui::EndPopup();
			}

			ImGui::EndPopup();
		}
	}

	void LevelViewportGuiLayer::DeleteSelectedActor()
	{
		Actor* SelectedActor = m_OwningLevelViewport->GetSelectedComponent() ?
		m_OwningLevelViewport->GetSelectedComponent()->GetOwningActor() : nullptr;

		if ( SelectedActor )
		{
			SelectedActor->Destroy();
		}
	}


	void LevelViewportGuiLayer::AlignSelectedComponentToSurfaceBelow()
	{
		SceneComponent* SelectedSceneComponent = static_cast<SceneComponent*>( m_OwningLevelViewport->GetSelectedComponent() );
		if ( SelectedSceneComponent )
		{
			std::vector<HitResult> Results;
			SelectedSceneComponent->GetWorld()->GetPhysicScene()->RaycastMulti(Results, SelectedSceneComponent->GetWorldLocation(), Vector::DownVector, 10000);

			for (uint32 i = 0; i < Results.size(); i++)
			{
				if ( Results[i].HitActor != SelectedSceneComponent->GetOwningActor() )
				{
					SelectedSceneComponent->SetWorldLocation(Results[i].Location);
					return;
				}
			}
		}
	}

}

#endif