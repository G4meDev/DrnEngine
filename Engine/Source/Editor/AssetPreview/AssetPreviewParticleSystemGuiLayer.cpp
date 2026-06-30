#include "DrnPCH.h"
#include "AssetPreviewParticleSystemGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Engine/PreviewWorld.h"
#include "Editor/EditorPanels/ViewportPanel.h"
#include "Editor/Editor.h"

namespace Drn
{
	AssetPreviewParticleSystemGuiLayer::AssetPreviewParticleSystemGuiLayer( ParticleSystem* InOwningAsset )
		: SelectedEmitterIndex(-1)
		, DeferrEmitterDeleteIndex(-1)
		, SelectedModuleIndex(-1)
		, DeferrModuleDeleteIndex(-1)
		, DeferrModuleEmitterDeleteIndex(-1)
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

		ParticlePreview = m_World->GetWorld()->SpawnActor<Particle>();
		ParticlePreview->GetParticleSystemComponenet()->SetTemplate(AssetHandle<ParticleSystem>(""));

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

		if (DeferrModuleDeleteIndex >= 0 && DeferrModuleEmitterDeleteIndex >= 0)
		{
			ParticleEmitter* Emitter = m_OwningAsset->Emitters[DeferrModuleEmitterDeleteIndex];

			EParticleModuleStage Stage = EParticleModuleStage::None;
			int32 InternalIndex = -1;
			GetModuleInternalIndex(Emitter, DeferrModuleDeleteIndex, Stage, InternalIndex);

			if (Stage == EParticleModuleStage::EmitterUpdate)
			{
				auto& Modules = Emitter->SpawningModules;
				Modules.erase(Modules.begin() + InternalIndex);
			}

			else if (Stage == EParticleModuleStage::ParticleSpawn)
			{
				auto& Modules = Emitter->SpawnModules;
				Modules.erase(Modules.begin() + InternalIndex);
			}

			else if (Stage == EParticleModuleStage::ParticleUpdate)
			{
				auto& Modules = Emitter->UpdateModules;
				Modules.erase(Modules.begin() + InternalIndex);
			}

			SelectedModuleIndex = -1;
			DeferrModuleDeleteIndex = -1;
			DeferrModuleEmitterDeleteIndex = -1;
		}

		if (DeferrEmitterDeleteIndex >= 0)
		{
			m_OwningAsset->Emitters.erase(m_OwningAsset->Emitters.begin() + DeferrEmitterDeleteIndex);

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
			m_OwningAsset->Save();
		} ImGui::SameLine();

		if (ImGui::Button("Add Emitter"))
		{
			m_OwningAsset->Emitters.push_back(new ParticleEmitter());
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
			ParticleEmitter* Emitter = m_OwningAsset->Emitters[SelectedEmitterIndex];

			const int32 EmitterNameCharacterLimit = 64;
			char EmitterName[EmitterNameCharacterLimit];
			strcpy_s(EmitterName, sizeof(EmitterName), Emitter->GetName().c_str());

			if ( ImGui::InputText( "## ", EmitterName, EmitterNameCharacterLimit) )
			{
				Emitter->SetName(EmitterName);
			}


		}
	}

	void AssetPreviewParticleSystemGuiLayer::DrawEmitters()
	{
		ImGui::ShowDemoWindow();

		for (int32 EmitterIndex = 0; EmitterIndex < m_OwningAsset->Emitters.size(); EmitterIndex++)
		{
			ParticleEmitter* Emitter = m_OwningAsset->Emitters[EmitterIndex];

			const bool bEmitterSelected = SelectedEmitterIndex == EmitterIndex;
			if (bEmitterSelected) { ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.2f, 0.3f, 0.4f, 1.0f ) ); }

			const std::string DisplayLabel = std::format("Emitter##{}", EmitterIndex);
			if (ImGui::BeginChild(DisplayLabel.c_str(), ImVec2(320, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
			{
				DrawEmitterHeader(EmitterIndex);
				DrawEmitterModules(EmitterIndex, bEmitterSelected);
			}
			ImGui::EndChild(); ImGui::SameLine();

			if (bEmitterSelected) { ImGui::PopStyleColor(); }

			if (ImGui::IsItemClicked() && (SelectedEmitterIndex != EmitterIndex))
			{
				SelectedEmitterIndex = EmitterIndex;
				SelectedModuleIndex = -1;
			}
		}
	}

	void AssetPreviewParticleSystemGuiLayer::DrawEmitterHeader( int32 Index )
	{
		auto& Emitters = m_OwningAsset->Emitters;
		ParticleEmitter* Emitter = m_OwningAsset->Emitters[Index];

		if (ImGui::Button("X"))
		{
			DeferrEmitterDeleteIndex = Index;
		} ImGui::SameLine();

		if (ImGui::Button(Emitter->IsEnabled() ? "O" : "-"))
		{
			Emitter->SetEnabled(!Emitter->IsEnabled());
		} ImGui::SameLine();

		if (ImGui::Button("Add Module"))
		{
			ImGui::OpenPopup("Add Popup");
		} ImGui::SameLine();

		ImGui::Text( Emitter->GetName().c_str() );

		ImGui::Separator();

		const char* StageDisplayNames[] = { "EmitterUpdate", "ParticleSpawn", "ParticleUpdate" };

		if (ImGui::BeginPopup("Add Popup"))
		{
			for (int32 ParticleStageIndex = 0; ParticleStageIndex < (int32)EParticleModuleStage::NumBits; ParticleStageIndex++)
			{
				if (ImGui::Button(StageDisplayNames[ParticleStageIndex]))
				{
					ImGui::OpenPopup(StageDisplayNames[ParticleStageIndex]);
				}

				if (ImGui::BeginPopup(StageDisplayNames[ParticleStageIndex]))
				{
					for (int32 CategoryIndex = 0; CategoryIndex < ParticleTypes::ParticleModuleCategories.size(); CategoryIndex++)
					{
						EParticleModuleStage Stage = EParticleModuleStage(1 << ParticleStageIndex);

						ParticleModuleCategory& Category = ParticleTypes::ParticleModuleCategories[CategoryIndex];
						std::string CategoryPopupStr = std::format("{}-{}", StageDisplayNames[ParticleStageIndex], Category.CategoryName);

						if (EnumHasAnyFlags(Category.ModulesSupportedStages, Stage) && ImGui::Button(Category.CategoryName.c_str()))
						{
							ImGui::OpenPopup(CategoryPopupStr.c_str());
						}

						if (ImGui::BeginPopup(CategoryPopupStr.c_str()))
						{
							for (int32 ParticleModuleIndex = 0; ParticleModuleIndex < (int32)EParticleModule::Max; ParticleModuleIndex++)
							{
								ParticleModuleMetaData& ModuleMetaData = ParticleTypes::ParticleModulesMetaData[ParticleModuleIndex];
								EParticleModuleStage Stage = EParticleModuleStage(1 << ParticleStageIndex);
							
								if (ModuleMetaData.IsParticleModuleSupportingStage(Stage))
								{
									if (ImGui::Button(ModuleMetaData.DisplayName.c_str()))
									{
										ParticleModule* CreateddModule = ParticleTypes::CreateParticleModule((EParticleModule)ParticleModuleIndex);
										if (Stage == EParticleModuleStage::EmitterUpdate)
										{
											Emitter->SpawningModules.push_back((ParticleModuleSpawn*)CreateddModule);
										}
										else if (Stage == EParticleModuleStage::ParticleSpawn)
										{
											Emitter->SpawnModules.push_back(CreateddModule);
										}
										else if (Stage == EParticleModuleStage::ParticleUpdate)
										{
											Emitter->UpdateModules.push_back(CreateddModule);
										}
										else
										{
											drn_check(false);
										}
							
										ImGui::CloseCurrentPopup();
									}
								}
							}

							ImGui::EndPopup();
						}
					}

					ImGui::EndPopup();
				}
			}

			ImGui::EndPopup();
		}
	}

	void AssetPreviewParticleSystemGuiLayer::DrawEmitterModules( int32 Index, bool EmitterSelected )
	{
		ParticleEmitter* Emitter = m_OwningAsset->Emitters[Index];

		auto DrawModule = [&](ParticleModule* Module, int32 StackIndex)
		{
			ParticleModuleMetaData& ModuleMetaData = ParticleTypes::ParticleModulesMetaData[(int32)Module->GetModuleType()];

			const bool bModuleSelected = EmitterSelected && (SelectedModuleIndex == StackIndex);
			if (bModuleSelected) { ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.2f, 0.8f, 0.4f, 1.0f ) ); }

			if (ImGui::BeginChild(std::format("Module##{}", StackIndex).c_str(), ImVec2(320, 32)))
			{
				if (ImGui::Button("x"))
				{
					DeferrModuleDeleteIndex = StackIndex;
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
				SelectedModuleIndex = StackIndex;
				SelectedEmitterIndex = Index;
			}
		};

		for (int32 ModuleIndex = 0; ModuleIndex < Emitter->SpawningModules.size(); ModuleIndex++)
		{
			DrawModule(Emitter->SpawningModules[ModuleIndex], GetModuleStackIndex(Emitter, EParticleModuleStage::EmitterUpdate, ModuleIndex));
		} ImGui::Separator();

		for (int32 ModuleIndex = 0; ModuleIndex < Emitter->SpawnModules.size(); ModuleIndex++)
		{
			DrawModule(Emitter->SpawnModules[ModuleIndex], GetModuleStackIndex(Emitter, EParticleModuleStage::ParticleSpawn, ModuleIndex));
		} ImGui::Separator();

		for (int32 ModuleIndex = 0; ModuleIndex < Emitter->UpdateModules.size(); ModuleIndex++)
		{
			DrawModule(Emitter->UpdateModules[ModuleIndex], GetModuleStackIndex(Emitter, EParticleModuleStage::ParticleUpdate, ModuleIndex));
		} ImGui::Separator();

		ImGui::Separator();
	}

	int32 AssetPreviewParticleSystemGuiLayer::GetModuleStackIndex( ParticleEmitter* Emitter, EParticleModuleStage Stage, int32 InternalIndex )
	{
		int32 StackIndex = InternalIndex;

		if (Stage >= EParticleModuleStage::ParticleSpawn)
		{
			StackIndex += Emitter->SpawningModules.size();
		}

		if (Stage >= EParticleModuleStage::ParticleUpdate)
		{
			StackIndex += Emitter->SpawnModules.size();
		}

		return StackIndex;
	}

	void AssetPreviewParticleSystemGuiLayer::GetModuleInternalIndex( ParticleEmitter* Emitter, int32 StackIndex, EParticleModuleStage& Stage, int32& InternalIndex )
	{
		const int32 SpawningSize = Emitter->SpawningModules.size();
		const int32 SpawnSize = Emitter->SpawnModules.size();
		const int32 UpdateSize = Emitter->UpdateModules.size();

		if (StackIndex < SpawningSize)
		{
			InternalIndex = StackIndex;
			Stage = EParticleModuleStage::EmitterUpdate;
		}

		else if (StackIndex < SpawningSize + SpawnSize)
		{
			InternalIndex = StackIndex - SpawningSize;
			Stage = EParticleModuleStage::ParticleSpawn;
		}

		else if (StackIndex < SpawningSize + SpawnSize + UpdateSize)
		{
			InternalIndex = StackIndex - SpawningSize - SpawnSize;
			Stage = EParticleModuleStage::ParticleUpdate;
		}

		else
		{
			drn_check(false);
		}
	}

}  // namespace Drn

#endif