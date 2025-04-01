#include "DrnPCH.h"
#include "WorldOutlinerGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/Renderer.h"
#include "Editor/Viewport/Viewport.h"

#include "imgui.h"

LOG_DEFINE_CATEGORY( LogWorldOutliner, "WorldOutliner" );

namespace Drn
{
	WorldOutlinerGuiLayer::WorldOutlinerGuiLayer()
	{
		
	}

	WorldOutlinerGuiLayer::~WorldOutlinerGuiLayer()
	{
		
	}

	void WorldOutlinerGuiLayer::Draw( float DeltaTime )
	{
		if (!Viewport::Get()->IsVisible())
		{
			return;
		}

		if (!ImGui::Begin("WorldOutliner"))
		{
			ImGui::End();
		}

		DrawActorList();

		ImGui::End();
	}

	void WorldOutlinerGuiLayer::DrawActorList()
	{
		World* W = GetMainWorld();

		ImGui::BeginChild( "WorldOutlinerActorList" );

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

		ImGui::EndChild();
	}

	World* WorldOutlinerGuiLayer::GetMainWorld()
	{
		return Renderer::Get()->MainWorld;
	}

}

#endif