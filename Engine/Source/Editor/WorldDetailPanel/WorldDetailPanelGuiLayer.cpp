#include "DrnPCH.h"
#include "WorldDetailPanelGuiLayer.h"

#if WITH_EDITOR

#include "Editor/Viewport/Viewport.h"
#include "imgui.h"

namespace Drn
{
	WorldDetailPanelGuiLayer::WorldDetailPanelGuiLayer()
	{
		
	}

	WorldDetailPanelGuiLayer::~WorldDetailPanelGuiLayer()
	{
		
	}

	void WorldDetailPanelGuiLayer::Draw( float DeltaTime )
	{
		if (!Viewport::Get()->IsVisible())
		{
			return;
		}

		if ( !ImGui::Begin( "Detail") )
		{
			ImGui::End();
		}



		ImGui::End();
	}

}

#endif