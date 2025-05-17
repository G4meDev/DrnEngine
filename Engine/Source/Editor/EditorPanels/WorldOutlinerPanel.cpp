#include "DrnPCH.h"
#include "WorldOutlinerPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/Renderer.h"

#include "imgui.h"

LOG_DEFINE_CATEGORY( LogWorldOutliner, "WorldOutliner" );

namespace Drn
{
	WorldOutlinerPanel::WorldOutlinerPanel(World* InWorld)
		: m_World(InWorld)
		, m_ShowTransient(false)
	{
	}

	WorldOutlinerPanel::~WorldOutlinerPanel()
	{
		
	}

	void WorldOutlinerPanel::Draw( float DeltaTime )
	{
		SCOPE_STAT( WorldOutlinerPanelDraw );

		DrawMenu(DeltaTime);

		ImGui::Separator();

		Component* SelectedComponent = nullptr;
		if (GetSelectedComponentDel.IsBound())
		{
			SelectedComponent = GetSelectedComponentDel.Execute();
		}

		int i = 0;
		for (Actor* actor : m_World->GetActorList())
		{
			if (!m_ShowTransient && actor->IsTransient())
			{
				continue;
			}

			else if (actor->IsTransient())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
			}

			std::string ActorLabel = actor->GetActorLabel();

			ImGui::PushID(i++);

			const bool ActorIsSelected = SelectedComponent ? SelectedComponent->GetOwningActor() == actor : false;
			if ( ImGui::Selectable(ActorLabel.c_str(), ActorIsSelected) )
			{
				if (!ActorIsSelected)
				{
					OnSelectedNewComponent.Braodcast( actor->GetRoot() );
				}
			}

			if ( ImGui::BeginPopupContextItem( "Actor pop up") )
			{
				if (ImGui::BeginMenu("Edit"))
				{
					if (ImGui::Button("Delete"))
					{
						DeleteActor(actor);
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			ImGui::PopID();

			if (actor->IsTransient())
			{
				ImGui::PopStyleColor();
			}
		}
	}

	void WorldOutlinerPanel::DrawMenu( float DeltaTime )
	{
		ImGui::Checkbox( "Transient", &m_ShowTransient);
	}

	void WorldOutlinerPanel::DeleteActor( Actor* actor )
	{
		if (actor)
		{
			LOG( LogWorldOutliner, Info, "removing actor \"%s\"", actor->GetActorLabel().c_str());
			ImGui::CloseCurrentPopup();

			actor->Destroy();
		}
	}

}

#endif