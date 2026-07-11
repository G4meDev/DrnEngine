#include "DrnPCH.h"
#include "AssetPreviewParticleSystemGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Engine/PreviewWorld.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/Editor.h"

namespace Drn
{
	ImVec4 GetSeededRandomColor(int32 Seed)
	{
		Vector RandomVector = RandomStream(Seed).GetUnitVector() * 0.5f + 0.5f;
		return ImVec4(RandomVector.GetX(), RandomVector.GetY(), RandomVector.GetZ(), 1.0f);
	}

	AssetPreviewParticleSystemGuiLayer::AssetPreviewParticleSystemGuiLayer( ParticleSystem* InOwningAsset )
		: SelectedEmitterIndex(-1)
		, DeferrEmitterDeleteIndex(-1)
		, SelectedModuleIndex(-1)
		, DeferrModuleDeleteIndex(-1)
		, DeferrModuleEmitterDeleteIndex(-1)
	{
		m_OwningAsset = AssetHandle<ParticleSystem>( InOwningAsset->m_Path );
		m_OwningAsset.Load();

		const std::string TransientPath = Path::ToTransientPath(InOwningAsset->m_Path);
		drn_check(FileSystem::CopyFile(Path::ConvertProjectPath(TransientPath), Path::ConvertProjectPath(InOwningAsset->m_Path), true, true));

		TransientAsset = AssetHandle<ParticleSystem>( TransientPath );
		TransientAsset.Load();

		m_World = new PreviewWorld;
		//m_World->GetWorld()->SetGameMode(true);

		m_World->SkyLight->SetIntensity(0.4f);

		m_World->DirectionalLight->SetIntensity(1);
		m_World->DirectionalLight->SetActorRotation(Quat(0, XM_PIDIV4, XM_PI));

		Quat CameraRotation(0, Math::PI / 4, Math::PI * 5 / 4);
		m_World->GetWorld()->GetViewportCamera()->SetActorRotation( CameraRotation );

		ParticlePreview = m_World->GetWorld()->SpawnActor<Particle>();
		//ParticlePreview->GetParticleSystemComponenet()->SetTemplate(AssetHandle<ParticleSystem>(""));
		ParticlePreview->GetParticleSystemComponenet()->SetTemplate(m_OwningAsset);

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
		std::string name = TransientAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (DeferrModuleDeleteIndex >= 0 && DeferrModuleEmitterDeleteIndex >= 0)
		{
			ParticleEmitter* Emitter = TransientAsset->Emitters[DeferrModuleEmitterDeleteIndex];
			auto& Modules = Emitter->Modules;
			Modules.erase(Modules.begin() + DeferrModuleDeleteIndex);

			SelectedModuleIndex = -1;
			DeferrModuleDeleteIndex = -1;
			DeferrModuleEmitterDeleteIndex = -1;
		}

		if (DeferrEmitterDeleteIndex >= 0)
		{
			TransientAsset->Emitters.erase(TransientAsset->Emitters.begin() + DeferrEmitterDeleteIndex);

			SelectedEmitterIndex = -1;
			DeferrEmitterDeleteIndex = -1;
		}

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
			m_ViewportPanel->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

		if (ImGui::Button("Save"))
		{
			OnSave();
		} ImGui::SameLine();

		if (ImGui::Button("Deactivate"))
		{
			ParticlePreview->GetParticleSystemComponenet()->Deactivate();
		} ImGui::SameLine();

		if (ImGui::Button("Activate"))
		{
			ParticlePreview->GetParticleSystemComponenet()->Activate();
		} ImGui::SameLine();

		if (ImGui::Button("Add Emitter"))
		{
			TransientAsset->Emitters.push_back(new ParticleEmitter());
		}

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

				if (ImGui::BeginTabItem("Module"))
				{
					DrawModuleParams();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Stats"))
				{
					DrawStats();
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild(); ImGui::SameLine();

		if ( ImGui::BeginChild( "Emitters", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar ) )
		{
			DrawEmitters();
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

	void AssetPreviewParticleSystemGuiLayer::DrawEmitterParams()
	{
		if (SelectedEmitterIndex >= 0)
		{
			ParticleEmitter* Emitter = TransientAsset->Emitters[SelectedEmitterIndex];
			Emitter->Draw();
		}
	}

	void AssetPreviewParticleSystemGuiLayer::DrawModuleParams()
	{
		if (SelectedEmitterIndex >= 0 && SelectedModuleIndex >= 0)
		{
			ParticleEmitter* Emitter = TransientAsset->Emitters[SelectedEmitterIndex];
			Emitter->Modules[SelectedModuleIndex]->Draw(Emitter);
		}
	}

	void AssetPreviewParticleSystemGuiLayer::DrawEmitters()
	{
		for (int32 EmitterIndex = 0; EmitterIndex < TransientAsset->Emitters.size(); EmitterIndex++)
		{
			ParticleEmitter* Emitter = TransientAsset->Emitters[EmitterIndex];

			if (ImGui::BeginChild(std::format("Ab##{}", EmitterIndex).c_str(), ImVec2(320, 0), ImGuiChildFlags_NavFlattened))
			{
				{
					ImGui::PushStyleColor( ImGuiCol_ChildBg, GetSeededRandomColor(EmitterIndex));
					ImGui::BeginChild(std::format("Color##{}", EmitterIndex).c_str(), ImVec2(0, 10), ImGuiChildFlags_NavFlattened);
					ImGui::EndChild();
					ImGui::PopStyleColor();
				}

				const bool bEmitterSelected = SelectedEmitterIndex == EmitterIndex;
				if (bEmitterSelected) { ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.2f, 0.3f, 0.4f, 1.0f ) ); }

				const std::string DisplayLabel = std::format("Emitter##{}", EmitterIndex);
				if (ImGui::BeginChild(DisplayLabel.c_str(), ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
				{
					DrawEmitterHeader(EmitterIndex);
					DrawEmitterModules(EmitterIndex, bEmitterSelected);
				}
				ImGui::EndChild();

				if (bEmitterSelected) { ImGui::PopStyleColor(); }

				if (ImGui::IsItemClicked() && (SelectedEmitterIndex != EmitterIndex))
				{
					SelectedEmitterIndex = EmitterIndex;
					SelectedModuleIndex = -1;
				}
			}

			ImGui::EndChild(); ImGui::SameLine();
		}
	}

	void AssetPreviewParticleSystemGuiLayer::DrawEmitterHeader( int32 Index )
	{
		auto& Emitters = TransientAsset->Emitters;
		ParticleEmitter* Emitter = TransientAsset->Emitters[Index];

		if (ImGui::Button("X"))
		{
			DeferrEmitterDeleteIndex = Index;
		} ImGui::SameLine();

		if (ImGui::Button(Emitter->IsEnabled() ? "O" : "-"))
		{
			Emitter->SetEnabled(!Emitter->IsEnabled());
		} ImGui::SameLine();

		ImGui::Text( Emitter->GetName().c_str() );

		ImGui::Separator();

		if (ImGui::BeginMenu("Add Module"))
		{
			for (int32 CategoryIndex = 0; CategoryIndex < ParticleTypes::ParticleModuleCategories.size(); CategoryIndex++)
			{
				ParticleModuleCategory& Category = ParticleTypes::ParticleModuleCategories[CategoryIndex];
				std::string CategoryPopupStr = Category.CategoryName;

				if (ImGui::BeginMenu(Category.CategoryName.c_str()))
				{
					for (EParticleModule SubCategoryModule : Category.Modules)
					{
						uint32 ParticleModuleIndex = (uint32)SubCategoryModule;
						ParticleModuleMetaData& ModuleMetaData = ParticleTypes::ParticleModulesMetaData[ParticleModuleIndex];
						if (ImGui::Button(ModuleMetaData.DisplayName.c_str()))
						{
							ParticleModule* CreateddModule = ParticleTypes::CreateParticleModule((EParticleModule)ParticleModuleIndex);
							Emitter->Modules.push_back(CreateddModule);
						}
					}

					ImGui::EndMenu();
				}
			}

			ImGui::EndMenu();
		}

		//draw_list->AddRectFilled(box_min, box_max, *(ImU32*)(&HeaderColor));
	}

	void AssetPreviewParticleSystemGuiLayer::DrawEmitterModules( int32 Index, bool EmitterSelected )
	{
		ParticleEmitter* Emitter = TransientAsset->Emitters[Index];

		for (int32 ModuleIndex = 0; ModuleIndex < Emitter->Modules.size(); ModuleIndex++)
		{
			ParticleModule* Module = Emitter->Modules[ModuleIndex];
			ParticleModuleMetaData& ModuleMetaData = ParticleTypes::ParticleModulesMetaData[(int32)Module->GetModuleType()];

			const bool bModuleSelected = EmitterSelected && (SelectedModuleIndex == ModuleIndex);
			if (bModuleSelected) { ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.2f, 0.8f, 0.4f, 1.0f ) ); }

			if (ImGui::BeginChild(std::format("Module##{}", ModuleIndex).c_str(), ImVec2(320, 32)))
			{
				if (ImGui::Button("x"))
				{
					DeferrModuleDeleteIndex = ModuleIndex;
					DeferrModuleEmitterDeleteIndex = Index;
				} ImGui::SameLine();

				if (ImGui::Button(Module->IsEnabled() ? "O" : "-"))
				{
					Module->SetEnabled(!Module->IsEnabled());
				} ImGui::SameLine();

				ImGui::Text(ModuleMetaData.DisplayName.c_str());
			}
			ImGui::EndChild();

			if (bModuleSelected) { ImGui::PopStyleColor(); }

			if (ImGui::IsItemClicked())
			{
				SelectedModuleIndex = ModuleIndex;
				SelectedEmitterIndex = Index;
			}
		}

		ImGui::Separator();
	}

	void AssetPreviewParticleSystemGuiLayer::OnSave()
	{
		TransientAsset->Save();
		drn_check(FileSystem::CopyFile(Path::ConvertProjectPath(m_OwningAsset->m_Path), Path::ConvertProjectPath(TransientAsset->m_Path), true, true));

		m_OwningAsset->Load();
		Editor::Get()->NotifyParticleReimported(m_OwningAsset);
	}

#define BEGIN_DRAW_PARTICLE_STAT()																								 	\
	for (int32 EmitterIndex = 0; EmitterIndex < ParticlePreview->GetParticleSystemComponenet()->Emitters.size();EmitterIndex++) {	\
		ParticleEmitterInstance* Instance = ParticlePreview->GetParticleSystemComponenet()->Emitters[EmitterIndex];					\
		if (Instance && Instance->Emitter) {																						\
			ImGui::PushID(EmitterIndex);ImGui::PushStyleColor(ImGuiCol_Text, GetSeededRandomColor(EmitterIndex));					

#define END_DRAW_PARTICLE_STAT()																								 	\
	ImGui::PopStyleColor(1); ImGui::PopID(); } }

	void AssetPreviewParticleSystemGuiLayer::DrawStats()
	{
		const bool bCompleted = ParticlePreview->GetParticleSystemComponenet()->bWasCompleted;
		ImGui::Text(bCompleted ? "Completed" : "Playing");
		ImGui::Separator();

		BEGIN_DRAW_PARTICLE_STAT();
		ImGui::Text("%i/ %i", Instance->ActiveParticles, Instance->MaxActiveParticles);
		END_DRAW_PARTICLE_STAT();

		ImGui::Separator();

		BEGIN_DRAW_PARTICLE_STAT();
		ImGui::Text("%i/ %.2f/ %.2f", Instance->LoopCount, Instance->EmitterTime, Instance->SecondsSinceCreation);
		END_DRAW_PARTICLE_STAT();

		ImGui::Separator();

		BEGIN_DRAW_PARTICLE_STAT();
		int32 ParticleMemorySize = (Instance->ParticleStride + sizeof(uint16)) * Instance->MaxActiveParticles;
		int32 InstanceMemorySize = Instance->InstancePayloadSize;
		ImGui::Text("%i bytes/ %i bytes", ParticleMemorySize, InstanceMemorySize);
		END_DRAW_PARTICLE_STAT();
	}

        }  // namespace Drn

#endif