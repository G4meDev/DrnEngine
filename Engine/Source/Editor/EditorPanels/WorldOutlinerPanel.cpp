#include "DrnPCH.h"
#include "WorldOutlinerPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/Renderer.h"
#include "Editor/Editor.h"
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
		SCOPE_STAT();

		DrawMenu(DeltaTime);

		ImGui::Separator();

		Component* SelectedComponent = nullptr;
		if (GetSelectedComponentDel.IsBound())
		{
			SelectedComponent = GetSelectedComponentDel.Execute();
		}

		std::vector<Actor*> SortedActors;
		for (Actor* actor : m_World->GetActorList())
		{
			if (!actor->IsTransient() || (actor->IsTransient() && m_ShowTransient))
			{
				SortedActors.push_back(actor);
			}
		}

		std::sort( SortedActors.begin(), SortedActors.end(), []( Actor* A, Actor* B ) { return A->GetActorLabel() < B->GetActorLabel(); } );
		for (int32 ActorIndex = 0; ActorIndex < SortedActors.size(); ActorIndex++)
		{
			Actor* actor = SortedActors[ActorIndex];
			if (actor->IsTransient())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
			}

			std::string ActorLabel = actor->GetActorLabel();
			ImGui::PushID(ActorIndex);

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

					if (ImGui::Button("Duplicate"))
					{
						DuplicateActor(actor);
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
		ImGui::CloseCurrentPopup();
		Editor::Get()->DeleteActor(actor);
	}

	void WorldOutlinerPanel::DuplicateActor( Actor* actor )
	{
		ImGui::CloseCurrentPopup();
		Editor::Get()->DuplicateActor(actor);
	}

}

#endif