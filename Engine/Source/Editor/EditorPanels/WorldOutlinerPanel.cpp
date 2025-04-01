#include "DrnPCH.h"
#include "WorldOutlinerPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/Renderer.h"

#include "imgui.h"

namespace Drn
{
	WorldOutlinerPanel::WorldOutlinerPanel(World* InWorld)
		: m_World(InWorld)
	{
		
	}

	WorldOutlinerPanel::~WorldOutlinerPanel()
	{
		
	}

	void WorldOutlinerPanel::Draw( float DeltaTime )
	{
		int i = 0;

		for (const Actor* Actor : m_World->GetActorList())
		{
			std::string ActorLabel = Actor->GetActorLabel();

			ImGui::PushID(i++);

			if ( ImGui::Selectable(ActorLabel.c_str(), SelectedActor == Actor) )
			{
				SelectedActor = Actor;
			}

			ImGui::PopID();
		}
	}

}

#endif