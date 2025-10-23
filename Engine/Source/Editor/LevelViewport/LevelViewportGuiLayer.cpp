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

		if (bLeftPanel)
		{
			if (ImGui::BeginChild("Mode", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
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
					std::string StringPayload = std::string(static_cast<const char*>(Payload->Data));
					BufferArchive* Ar = new BufferArchive(StringPayload.size() + 8, false);
					*Ar << StringPayload;
					Ar->ResetPointer();

					m_ViewportPanel->GetSceneRenderer()->QueueScreenReprojection(Editor::GetScreenPositionRelative(), this, &LevelViewportGuiLayer::OnScreenReprojectDropAsset, Ar);
				}

				else if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_EditorSpawnable()))
				{
					BufferArchive* Ar = new BufferArchive(8, false);
					uint64 Ptr = *reinterpret_cast<uint64*>(Payload->Data);
					*Ar << Ptr;
					Ar->ResetPointer();

					m_ViewportPanel->GetSceneRenderer()->QueueScreenReprojection(Editor::GetScreenPositionRelative(), this, &LevelViewportGuiLayer::OnScreenReprojectDropEditorActor, Ar);
				}

				ImGui::EndDragDropTarget();
			}

			DrawContextPopup();

		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (m_ShowDetail)
		{
			if (ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
			{
				m_ActorDetailPanel->Draw(DeltaTime);
			}
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
				
				if ( ImGui::MenuItem( "Reimport materials" ) )
				{
					Editor::Get()->ReimportMaterials();
				}

				if ( ImGui::MenuItem( "Reimport static meshes" ) )
				{
					Editor::Get()->ReimportStaticMeshes();
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
				auto DrawViewFlag = [&](EWorldViewFlag Flag, const char* Name)
				{
					bool ViewHasFlag = m_OwningLevelViewport->m_OwningWorld->HasViewFlag(Flag);
					if (ImGui::MenuItem( Name, NULL, &ViewHasFlag))
					{
						m_OwningLevelViewport->m_OwningWorld->SetViewFlag(Flag, ViewHasFlag);
					}
				};

				DrawViewFlag( EWorldViewFlag::Collision, "Collision" );
				DrawViewFlag( EWorldViewFlag::Light, "Light" );

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

				DrawBufferVisualizationMenu();

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

	void LevelViewportGuiLayer::DrawBufferVisualizationMenu()
	{
		if (World* OwningWorld = m_OwningLevelViewport->m_OwningWorld)
		{
			const char* BufferVisualizations[] = { "Final Image", "Base Color", "Metallic", "Roughness", "WorldNormal", "MaterialAO", "ScreenSpaceAO", "CombinedAO", "Depth",
				"SubsurfaceColor", "ShadingModel", "Velocity", "PreTonemapColor", "LinearDepth", "Bloom", "ScreenSpaceReflection" };
			int CurrentBufferVisualization = static_cast<int>(OwningWorld->GetBufferVisualization());
			if ( ImGui::Combo( "Buffer Visualization", &CurrentBufferVisualization, BufferVisualizations, IM_ARRAYSIZE( BufferVisualizations )))
			{
				OwningWorld->SetBufferVisualization(static_cast<EBufferVisualization>(CurrentBufferVisualization));
			}
		}
	}

	void LevelViewportGuiLayer::HandleViewportSpawnAsset( const std::string& AssetPath, const Vector& WorldPosition )
	{
		AssetHandle<Asset> asset(AssetPath);
		EAssetType Type = asset.LoadGeneric();

		if (asset.IsValid() && Type == EAssetType::StaticMesh)
		{
			StaticMeshActor* NewActor = WorldManager::Get()->GetMainWorld()->SpawnActor<StaticMeshActor>();

			AssetHandle<StaticMesh> MeshAsset(AssetPath);
			MeshAsset.Load();
			NewActor->GetMeshComponent()->SetMesh(MeshAsset);
			NewActor->SetActorLocation(WorldPosition);
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

	void LevelViewportGuiLayer::HandleViewportSpawnEditorActor( EditorLevelSpawnable* EditorActor, const Vector& WorldPosition )
	{
		if (EditorActor)
		{
			Actor* SpawnActor = (EditorActor)->SpawnFunc(m_OwningLevelViewport->m_OwningWorld);
			SpawnActor->SetActorLocation(WorldPosition);
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

	void LevelViewportGuiLayer::OnScreenReprojectDropAsset( bool bHit, const Vector& WorldLocation, void* Payload )
	{
		BufferArchive* Ar = static_cast<BufferArchive*>(Payload);
		std::string StringPayload;
		*Ar >> StringPayload;

		HandleViewportSpawnAsset(StringPayload, WorldLocation);
	}

	void LevelViewportGuiLayer::OnScreenReprojectDropEditorActor( bool bHit, const Vector& WorldLocation, void* Payload )
	{
		BufferArchive* Ar = static_cast<BufferArchive*>(Payload);
		uint64 Ptr;
		*Ar >> Ptr;
		EditorLevelSpawnable* Spawnable = reinterpret_cast<EditorLevelSpawnable*>( Ptr );

		HandleViewportSpawnEditorActor(Spawnable, WorldLocation);
	}

}

#endif