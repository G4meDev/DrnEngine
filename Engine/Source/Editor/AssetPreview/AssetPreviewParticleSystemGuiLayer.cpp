#include "DrnPCH.h"
#include "AssetPreviewParticleSystemGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Engine/PreviewWorld.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/Editor.h"

namespace Drn
{
	AssetPreviewParticleSystemGuiLayer::AssetPreviewParticleSystemGuiLayer( ParticleSystem* InOwningAsset )
	{
		m_OwningAsset = AssetHandle<ParticleSystem>( InOwningAsset->m_Path );
		m_OwningAsset.Load();

		m_World = new PreviewWorld;
		//m_World->GetWorld()->SetGameMode(true);

		m_World->SkyLight->SetIntensity(0.4f);

		m_World->DirectionalLight->SetIntensity(1);
		m_World->DirectionalLight->SetActorRotation(Quat(0, XM_PIDIV4, XM_PI));

		Quat CameraRotation(0, Math::PI / 4, Math::PI * 5 / 4);
		m_World->GetWorld()->GetViewportCamera()->SetActorRotation( CameraRotation );

		//Vector CameraPosition = StaticMeshAsset->GetBounds().Origin + CameraRotation.GetAxisZ() * StaticMeshAsset->GetBounds().SphereRadius * -7;
		//TargetWorld->GetWorld()->GetViewportCamera()->SetActorLocation( CameraPosition );

		//m_World->GetSceneRenderer()->ResizeViewDeferred(IntPoint(THUMBNAIL_TEXTURE_SIZE));

		m_ViewportPanel = std::make_unique<ViewportPanel>( m_World->GetScene() );
	}

	AssetPreviewParticleSystemGuiLayer::~AssetPreviewParticleSystemGuiLayer()
	{
		m_World = nullptr;
		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewParticleSystemGuiLayer::Draw( float DeltaTime )
	{
		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
			m_ViewportPanel->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

		if (ImGui::Button("Save"))
		{
			m_OwningAsset->Save();
		}

		ImGui::InputFloat( "Test", &m_OwningAsset->Unused );

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );

		if (ImGui::BeginChild( "SidePanel", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			DrawViewport(DeltaTime);

			if ( ImGui::BeginTabBar( "Tab" ))
			{
				if (ImGui::BeginTabItem("Particle"))
				{
					DrawParticleParams();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Emitter"))
				{
					DrawEmitterParams();
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild(); ImGui::SameLine();

		if ( ImGui::BeginChild( "Emitters", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{

		}
		ImGui::EndChild();

		ImGui::End();
	}

	void AssetPreviewParticleSystemGuiLayer::DrawViewport(float DeltaTime)
	{
		ImVec2 ViewportSize = ImVec2( Editor::Get()->SidePanelSize, Editor::Get()->SidePanelSize );
		if (ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			m_ViewportPanel->SetRenderingEnabled(true);
			m_ViewportPanel->Draw(DeltaTime);
		}
		ImGui::EndChild();
	}

	void AssetPreviewParticleSystemGuiLayer::DrawParticleParams() {}

	void AssetPreviewParticleSystemGuiLayer::DrawEmitterParams() {}

        }  // namespace Drn

#endif