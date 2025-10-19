#include "DrnPCH.h"
#include "ModesPanel.h"

#if WITH_EDITOR

#include "Editor/Misc/EditorMisc.h"
#include "Editor/EditorConfig.h"
#include <imgui.h>

namespace Drn
{
	ModesPanel::ModesPanel()
		: m_SelectedCateory("All")
	{
		
	}

	ModesPanel::~ModesPanel() 
	{
		
	}

	void ModesPanel::Draw( float DeltaTime )
	{
		const uint64 CatorySizeX = ImGui::GetContentRegionAvail().x * 0.35f;
		if ( ImGui::BeginChild( "Category", ImVec2(CatorySizeX, -1), ImGuiChildFlags_Border) )
		{
			for (const std::string& Category : EditorMisc::Get()->EditorLevelSpawnablesCategories)
			{
				ImVec4 ButtonColor = (Category == m_SelectedCateory) ? ImColor( 0.3f, 0.3f, 0.3f ) : ImColor( 0.1f, 0.1f, 0.1f );
				ImGui::PushStyleColor( ImGuiCol_Button, ButtonColor);

				if ( ImGui::Button( Category.c_str(), ImVec2( -1, 0 ) ) )
				{
					m_SelectedCateory = Category;
				}

				ImGui::PopStyleColor();
			}

		}
		ImGui::EndChild();

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Child", ImVec2(0, 0), ImGuiChildFlags_Border) )
		{
			ImGui::PushStyleColor( ImGuiCol_Button, (ImVec4)ImColor( 0.1f, 0.1f, 0.1f) );

			for (EditorLevelSpawnable& Spawnable : EditorMisc::Get()->EditorLevelSpawnables)
			{
				if ( m_SelectedCateory == "All" || m_SelectedCateory == Spawnable.Category )
				{
					if (ImGui::Button(Spawnable.Name.c_str(), ImVec2(-1, 0)))
					{
						
					}

					if (ImGui::BeginDragDropSource())
					{
						if ( ImGui::GetDragDropPayload() == NULL )
						{
							EditorLevelSpawnable* Data = &Spawnable;
							ImGui::SetDragDropPayload( EditorConfig::Payload_EditorSpawnable(), &Data, sizeof(EditorLevelSpawnable*));
						}

						ImGui::Text( "%s", Spawnable.Name.c_str());

						ImGui::EndDragDropSource();
					}

					if (ImGui::IsItemHovered())
					{
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
					}
				}
			}

			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
	}

}

#endif