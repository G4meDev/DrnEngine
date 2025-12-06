#include "DrnPCH.h"
#include "StatsWindowGuiLayer.h"

#if WITH_EDITOR
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	StatsWindowGuiLayer::StatsWindowGuiLayer()
	{
		
	}

	StatsWindowGuiLayer::~StatsWindowGuiLayer()
	{
		
	}

	void StatsWindowGuiLayer::Draw( float DeltaTime )
	{
		SCOPE_STAT();

		if (!ImGui::Begin("Stats", (bool*)0, ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar))
		{
			ImGui::End();
			return;
		}

		for (int32 i = 0; i < (uint8)TextureStats::ETextureMemoryStatGroups::Max; i++)
		{
			TextureStats::ETextureMemoryStatGroups Group = (TextureStats::ETextureMemoryStatGroups)i;
			std::string msg = std::format("{} {}", TextureStats::GetTextureStatName(Group), TextureStats::GetTextureStatSize(Group));

			ImGui::Text( msg.c_str() );
		}

		ImGui::End();
	}
}

#endif