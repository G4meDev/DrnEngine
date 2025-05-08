#include "DrnPCH.h"
#include "WorldOutlinerPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/Renderer.h"

#include "imgui.h"

namespace Drn
{
	WorldOutlinerPanel::WorldOutlinerPanel(World* InWorld)
		: m_World(InWorld)
		, m_ShowTransient(false)
		, m_SelectedActor(nullptr)
	{
		InWorld->BindOnRemoveActor(std::bind(&WorldOutlinerPanel::OnRemovedActorFromWorld, this, std::placeholders::_1));
	}

	WorldOutlinerPanel::~WorldOutlinerPanel()
	{
		
	}

	void WorldOutlinerPanel::Draw( float DeltaTime )
	{
		SCOPE_STAT( WorldOutlinerPanelDraw );

		DrawMenu(DeltaTime);

		ImGui::Separator();

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

			if ( ImGui::Selectable(ActorLabel.c_str(), m_SelectedActor == actor) )
			{
				m_SelectedActor = actor;
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
			std::cout << actor->GetActorLabel().c_str();
			ImGui::CloseCurrentPopup();

			actor->Destroy();
		}
	}

	void WorldOutlinerPanel::OnRemovedActorFromWorld( const Actor* RemovedActor )
	{
		if (m_SelectedActor == RemovedActor)
		{
			m_SelectedActor = nullptr;
		}
	}

}

#endif