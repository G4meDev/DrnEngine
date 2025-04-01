#include "DrnPCH.h"
#include "WorldOutlinerPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/Renderer.h"
#include "Editor/Viewport/Viewport.h"

#include "imgui.h"

namespace Drn
{
	WorldOutlinerPanel::WorldOutlinerPanel()
	{
		
	}

	WorldOutlinerPanel::~WorldOutlinerPanel()
	{
		
	}

	void WorldOutlinerPanel::Draw( float DeltaTime )
	{
		World* W = GetMainWorld();

		int i = 0;

		for (const Actor* Actor : W->GetActorList())
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

	World* WorldOutlinerPanel::GetMainWorld()
	{
		return Renderer::Get()->MainWorld;
	}

}

#endif