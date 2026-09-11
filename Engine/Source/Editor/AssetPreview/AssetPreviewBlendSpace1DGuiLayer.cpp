#include "DrnPCH.h"
#include "AssetPreviewBlendSpace1DGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"
#include "Editor/FileImportMenu/FileImportMenu.h"
#include "Editor/EditorPanels/ViewportPanel.h"

namespace Drn
{
	AssetPreviewBlendSpace1DGuiLayer::AssetPreviewBlendSpace1DGuiLayer( BlendSpace1D* InOwningAsset )
		: m_ShowSceneSetting(true)
		, m_ShowDetail(true)
	{
		m_OwningAsset = AssetHandle<BlendSpace1D>(InOwningAsset->m_Path);
		m_OwningAsset.Load();

		PreviewWorld = WorldManager::Get()->AllocateWorld();
		PreviewWorld->SetTransient(true);
		PreviewWorld->SetPaused(true);
		PreviewWorld->SetEditorWorld();

		PreviewMesh = PreviewWorld->SpawnActor<SkeletalMeshActor>();
		PreviewMesh->GetMeshComponent()->SetMesh(m_OwningAsset->OwningSkeleton);
		PreviewMesh->GetMeshComponent()->SetTickInEditor(true);

		m_SkyLight = PreviewWorld->SpawnActor<SkyLightActor>();
		m_SkyLight->SetIntensity(0.4f);

		m_DirectionalLight = PreviewWorld->SpawnActor<DirectionalLightActor>();
		m_DirectionalLight->SetIntensity(1);
		m_DirectionalLight->SetActorRotation(Quat(0, XM_PIDIV4, XM_PI));

		m_ViewportPanel = std::make_unique<ViewportPanel>(PreviewWorld->GetScene());

		TRefCountPtr<AnimatorBlendSpace1DPreview> NewAnimator = new AnimatorBlendSpace1DPreview(this);
		PreviewMesh->GetMeshComponent()->SetAnimator(NewAnimator);

		if (m_OwningAsset->GetSkeleton().IsValid())
		{
			Vector FocalPoint = m_OwningAsset->GetSkeleton()->GetBounds().Origin;
			Vector CameraLocation = FocalPoint + Vector::OneVector * m_OwningAsset->GetSkeleton()->GetBounds().SphereRadius * m_OwningAsset->GetSkeleton()->ThumbnailDistance;
			PreviewWorld->GetViewportCamera()->SetActorTransform(Transform(CameraLocation, Quat::LookAtRotation(CameraLocation, FocalPoint)));
		}
	}

	AssetPreviewBlendSpace1DGuiLayer::~AssetPreviewBlendSpace1DGuiLayer()
	{
		if (PreviewWorld)
		{
			PreviewWorld->Destroy();
		}

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewBlendSpace1DGuiLayer::Draw( float DeltaTime )
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

	void AssetPreviewBlendSpace1DGuiLayer::DrawMenu()
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

			ImGui::EndMainMenuBar();
		}
	}

	void AssetPreviewBlendSpace1DGuiLayer::DrawDetailPanel()
	{
		if (ImGui::Button( "save" ))
		{
			m_OwningAsset.Get()->Save();
		}

		ImGui::Separator();

		DrawSkeleton();
		ImGui::InputFloat("Interpolation Speed", &m_OwningAsset->InterplationSpeed);

		ImGui::SliderFloat("Preview Sample Time", &PreviewSampleTime, m_OwningAsset->GetRangeMin(), m_OwningAsset->GetRangeMax());

		DrawSamples();
	}

	void AssetPreviewBlendSpace1DGuiLayer::DrawSkeleton()
	{
		std::string AssetPath	= m_OwningAsset->GetSkeleton().GetPath();
		std::string AssetName	= Path::ConvertShortPath(AssetPath);
		AssetName				= Path::RemoveFileExtension(AssetName);
		AssetName				= AssetName == "" ? "None" : AssetName;

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, EditorConfig::AssetInputColor);
		ImGui::Text( "%s", AssetName.c_str() );
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
			{
				auto AssetPath = static_cast<const char*>(payload->Data);

				AssetHandle<Asset> NewAsset(AssetPath);
				EAssetType Type = NewAsset.LoadType();

				if (Type == EAssetType::SkeletalMesh && AssetPath != m_OwningAsset->GetSkeleton().GetPath())
				{
					m_OwningAsset->OwningSkeleton = AssetHandle<SkeletalMesh>(AssetPath);
					m_OwningAsset->OwningSkeleton.Load();

					m_OwningAsset->SampleData.clear();
					m_OwningAsset->EvalProperties();
				}
			}

			ImGui::EndDragDropTarget();
		}
	}

	void AssetPreviewBlendSpace1DGuiLayer::DrawSamples()
	{
		if (ImGui::CollapsingHeader("Samples", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool bDirty = false;

			if (ImGui::Button("Add"))
			{
				bDirty = true;
				m_OwningAsset->SampleData.push_back({});
			}

			if (ImGui::Button("Remove"))
			{
				if (m_OwningAsset->SampleData.size() > 0)
				{
					bDirty = true;
					m_OwningAsset->SampleData.pop_back();
				}
			}

			if (ImGui::Button("Clear"))
			{
				if (m_OwningAsset->SampleData.size() > 0)
				{
					bDirty = true;
					m_OwningAsset->SampleData.clear();
				}
			}

			for (int32 SampleIndex = 0; SampleIndex < m_OwningAsset->SampleData.size(); SampleIndex++)
			{
				ImGui::PushID(SampleIndex);

				ImGui::Text(std::to_string(SampleIndex).c_str());
				bDirty |= m_OwningAsset->SampleData[SampleIndex].Draw(m_OwningAsset.Get());
				ImGui::Separator();

				ImGui::PopID();
			}

			if (bDirty)
			{
				m_OwningAsset->EvalProperties();
			}
		}
	}

        }  // namespace Drn

#endif