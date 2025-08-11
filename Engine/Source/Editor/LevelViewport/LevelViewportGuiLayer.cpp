#include "DrnPCH.h"
#include "LevelViewportGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Engine/PointLightActor.h"
#include "Runtime/Engine/SpotLightActor.h"
#include "Runtime/Engine/DirectionalLightActor.h"
#include "Runtime/Engine/PostProcessVolume.h"

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#include "Editor/LevelViewport/LevelViewport.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/EditorPanels/WorldOutlinerPanel.h"
#include "Editor/EditorPanels/ActorDetailPanel.h"
#include <imgui.h>

namespace Drn
{
	LevelViewportGuiLayer::LevelViewportGuiLayer(LevelViewport* InOwningLevelViewport)
		: m_OwningLevelViewport(InOwningLevelViewport)
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

		bool bLeftPanel = m_ShowOutliner;
		bool bRightPanel = m_ShowDetail;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize) * (bLeftPanel + bRightPanel) , 0.0f );

		if ( m_ShowOutliner && ImGui::BeginChild( "World Outliner", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			m_WorldOutlinerPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();

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

				ImGui::EndDragDropTarget();
			}

			DrawContextPopup();
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (m_ShowDetail && ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			m_ActorDetailPanel->Draw(DeltaTime);
		
		}
		ImGui::EndChild();

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
		if (WorldManager::Get()->m_PlayInEditor)
		{
			if (ImGui::Button("End Play"))
			{
				WorldManager::Get()->EndPlayInEditor();
			}

			ImGui::SameLine();

			if (WorldManager::Get()->m_PlayInEditorPaused)
			{
				if (ImGui::Button("Unpause"))
				{
					WorldManager::Get()->SetPlayInEditorPaused(false);
				}
			}

			else
			{
				if (ImGui::Button("Pause"))
				{
					WorldManager::Get()->SetPlayInEditorPaused(true);
				}
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

	void LevelViewportGuiLayer::OnHitPlay()
	{
		
	}

	void LevelViewportGuiLayer::DrawContextPopup()
	{
		if ( ImGui::BeginPopup( "ContextMenuPopup" ) )
		{
			if (ImGui::Button("Add"))
			{
				ImGui::OpenPopup( "AddPopup" );
			}

			if (ImGui::BeginPopup( "AddPopup" ))
			{
				if (ImGui::Button( "PointLight" ))
				{
					AddPointLight();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::Button( "SpotLight" ))
				{
					AddSpotLight();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::Button( "DirectionalLight" ))
				{
					AddDirectionalLight();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::Button( "PostProcessVolume" ))
				{
					AddPostProcessVolume();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

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

	void LevelViewportGuiLayer::AddPointLight()
	{
		PointLightActor* LightActor = m_OwningLevelViewport->m_OwningWorld->SpawnActor<PointLightActor>();
		LightActor->SetActorLabel( "PointLight_00" );

	}

	void LevelViewportGuiLayer::AddSpotLight()
	{
		SpotLightActor* LightActor = m_OwningLevelViewport->m_OwningWorld->SpawnActor<SpotLightActor>();
		LightActor->SetActorLabel( "SpotLight_00" );
	}

	void LevelViewportGuiLayer::AddDirectionalLight()
	{
		DirectionalLightActor* LightActor = m_OwningLevelViewport->m_OwningWorld->SpawnActor<DirectionalLightActor>();
		LightActor->SetActorLabel( "DirectionalLight_00" );
	}

	void LevelViewportGuiLayer::AddPostProcessVolume()
	{
		PostProcessVolume* PostProcessActor = m_OwningLevelViewport->m_OwningWorld->SpawnActor<PostProcessVolume>();
		PostProcessActor->SetActorLabel( "PostProcessVolume" );
	}

}

#endif