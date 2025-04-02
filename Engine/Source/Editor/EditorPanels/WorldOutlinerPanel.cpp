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
		
	}

	WorldOutlinerPanel::~WorldOutlinerPanel()
	{
		
	}

	void WorldOutlinerPanel::Draw( float DeltaTime )
	{
		DrawMenu(DeltaTime);

		ImGui::Separator();

		int i = 0;

		for (Actor* Actor : m_World->GetActorList())
		{
			if (!m_ShowTransient && Actor->IsTransient())
			{
				continue;
			}

			else if (Actor->IsTransient())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
			}

			std::string ActorLabel = Actor->GetActorLabel();

			ImGui::PushID(i++);

			if ( ImGui::Selectable(ActorLabel.c_str(), m_SelectedActor == Actor) )
			{
				m_SelectedActor = Actor;
			}

			ImGui::PopID();

			if (Actor->IsTransient())
			{
				ImGui::PopStyleColor();
			}
		}
	}

	void WorldOutlinerPanel::DrawMenu( float DeltaTime )
	{
		ImGui::Checkbox( "Transient", &m_ShowTransient);
	}

}

#endif