#include "DrnPCH.h"
#include "StatsWindowGuiLayer.h"

#if WITH_EDITOR
#include "Runtime/Renderer/RenderTexture.h"
#include "Runtime/Renderer/RenderBuffer.h"

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

		ImGui::Text( std::format( "Total render resource count {}", RenderResource::GetTotalResourceCount() ).c_str() );

		ImGui::Separator();

		for (int32 i = 0; i < (uint8)TextureStats::ETextureMemoryStatGroups::Max; i++)
		{
			TextureStats::ETextureMemoryStatGroups Group = (TextureStats::ETextureMemoryStatGroups)i;
			const int32 Size = TextureStats::GetTextureStatSize(Group);
			const float SizeInMB = Size / 1024.f;
			std::string msg = std::format("{} {} mb", TextureStats::GetTextureStatName(Group), SizeInMB);

			ImGui::Text( msg.c_str() );
		}

		ImGui::Separator();

		for (int32 i = 0; i < (uint8)BufferStats::EBufferMemoryStatGroups::Max; i++)
		{
			BufferStats::EBufferMemoryStatGroups Group = (BufferStats::EBufferMemoryStatGroups)i;
			const int32 Size = BufferStats::GetBufferStatSize(Group);
			const float SizeInMB = Size / 1024.f / 1024.f;
			std::string msg = std::format("{} {} mb", BufferStats::GetBufferStatName(Group), SizeInMB);

			ImGui::Text( msg.c_str() );
		}

		ImGui::End();
	}
}

#endif