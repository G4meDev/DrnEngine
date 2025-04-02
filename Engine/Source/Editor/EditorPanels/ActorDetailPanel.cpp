#include "DrnPCH.h"
#include "ActorDetailPanel.h"

#if WITH_EDITOR

#include <imgui.h>

namespace Drn
{
	ActorDetailPanel::ActorDetailPanel()
		: m_SelectedActor(nullptr)
		, m_SelectedComponent(nullptr)
	{
		
	}

	ActorDetailPanel::~ActorDetailPanel()
	{
		
	}

	void ActorDetailPanel::Draw( float DeltaTime )
	{
		if (m_SelectedActor)
		{
			m_SelectedComponent = m_SelectedComponent ? m_SelectedComponent : m_SelectedActor->GetRoot();

			ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x );

			if ( ImGui::InputText( "## ", name, 32) )
			{
				m_SelectedActor->SetActorLabel(name);
			}

			if (!ImGui::IsItemFocused())
			{
				strcpy(name, m_SelectedActor->GetActorLabel().c_str());
			}

			ImGui::Separator();

			DrawSceneComponents(DeltaTime);
			DrawDetails(DeltaTime);
		}
	}

	void ActorDetailPanel::SetSelectedActor( Actor* SelectedActor )
	{
		m_SelectedActor = SelectedActor;
	}

	void ActorDetailPanel::DrawSceneComponents( float DeltaTime )
	{
		if (ImGui::BeginChild("SceneComponent", ImVec2(0, 200), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			ImGui::BeginGroup();

			if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
			{
				DrawNextSceneComponent(m_SelectedActor->GetRoot());
				ImGui::EndTable();
			}

			ImGui::EndGroup();
		}
		ImGui::EndChild();

	}

	void ActorDetailPanel::DrawNextSceneComponent( SceneComponent* Comp )
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID( Comp->GetComponentLabel().c_str() );

		ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
		tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick;
		tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

		if ( Comp == m_SelectedComponent)
			tree_flags |= ImGuiTreeNodeFlags_Selected;

		if (!Comp->HasChild())
		tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	
		ImGui::SetNextItemOpen( true, ImGuiCond_::ImGuiCond_FirstUseEver);
		bool node_open = ImGui::TreeNodeEx( "", tree_flags, "%s", Comp->GetComponentLabel().c_str());

		if (ImGui::IsItemFocused())
		{
			m_SelectedComponent = Comp;
		}

		if ( node_open )
		{
			for (std::shared_ptr<SceneComponent> child : Comp->GetChilds())
			{
				if (child)
				{
					DrawNextSceneComponent( child.get() );
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void ActorDetailPanel::DrawDetails( float DeltaTime )
	{
		if (ImGui::BeginChild("Details", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if (m_SelectedActor && m_SelectedComponent)
			{
				m_SelectedComponent->DrawDetailPanel(DeltaTime);
			}

			ImGui::EndChild();
		}
	}

}

 #endif