#include "DrnPCH.h"
#include "BodySetup.h"

#include "Runtime/Physic/PhysicManager.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	void BodySetup::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			m_AggGeo.Serialize(Ar);

			m_TriMeshes.clear();
			uint8 TriMeshesCount = 0;
			Ar >> TriMeshesCount;
			m_TriMeshes.resize(TriMeshesCount);
			for (uint8 i = 0; i < TriMeshesCount; i++) { m_TriMeshes[i].Serialize(Ar);}

			Ar >> m_UseTriMesh;
		}
		else
		{
			m_AggGeo.Serialize(Ar);

			Ar << static_cast<uint8>(m_TriMeshes.size());
			for (uint8 i = 0; i < m_TriMeshes.size(); i++) { m_TriMeshes[i].Serialize(Ar); }

			Ar << m_UseTriMesh;
		}
	}

#if WITH_EDITOR
	void BodySetup::DrawDetailPanel()
	{
		DrawSphereShapes();
		DrawBoxShapes();
		DrawCapsuleShapes();
	}

	void BodySetup::DrawSphereShapes()
	{
		if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
		{
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
			tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if ( ImGui::TreeNodeEx( "", tree_flags, "Sphere" ) )
			{
				for ( int i = 0; i < m_AggGeo.SphereElems.size(); i++ )
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::PushID( i );

					if ( ImGui::TreeNodeEx( "", tree_flags, "%i", i ) )
					{
						ImGui::Text( "Center" );
						ImGui::SameLine();
						m_AggGeo.SphereElems[i].Center.Draw("Center");

						ImGui::Text( "Radius" );
						ImGui::SameLine();
						ImGui::DragFloat("##Radius", &(m_AggGeo.SphereElems[i].Radius), 0.1f, 0, 0, "%.3f");

						ImGui::TreePop();
					}

					ImGui::PopID();
				}

				ImGui::TreePop();
			}

			ImGui::EndTable();
		}
	}

	void BodySetup::DrawBoxShapes()
	{
		if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
		{
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
			tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if ( ImGui::TreeNodeEx( "", tree_flags, "Box" ) )
			{
				for ( int i = 0; i < m_AggGeo.BoxElems.size(); i++ )
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::PushID( i );

					if ( ImGui::TreeNodeEx( "", tree_flags, "%i", i ) )
					{
						ImGui::Text( "Center" );
						ImGui::SameLine();
						m_AggGeo.BoxElems[i].Center.Draw("Center");

						ImGui::Text( "Rotation" );
						ImGui::SameLine();
						m_AggGeo.BoxElems[i].Rotation.Draw("Rotation");

						ImGui::Text( "Extent" );
						ImGui::SameLine();
						m_AggGeo.BoxElems[i].Extent.Draw("Scale");

						ImGui::TreePop();
					}

					ImGui::PopID();
				}

				ImGui::TreePop();
			}

			ImGui::EndTable();
		}
	}

	void BodySetup::DrawCapsuleShapes()
	{
		if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
		{
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
			tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if ( ImGui::TreeNodeEx( "", tree_flags, "Capsule" ) )
			{
				for ( int i = 0; i < m_AggGeo.CapsuleElems.size(); i++ )
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::PushID( i );

					if ( ImGui::TreeNodeEx( "", tree_flags, "%i", i ) )
					{
						ImGui::Text( "Center" );
						ImGui::SameLine();
						m_AggGeo.CapsuleElems[i].Center.Draw("Center");

						ImGui::Text( "Rotation" );
						ImGui::SameLine();
						m_AggGeo.CapsuleElems[i].Rotation.Draw("Rotation");

						ImGui::Text( "Radius" );
						ImGui::SameLine();
						ImGui::DragFloat("##Radius", &(m_AggGeo.CapsuleElems[i].Radius), 0.3f, 0, 0, "%.3f");

						ImGui::Text( "Length" );
						ImGui::SameLine();
						ImGui::DragFloat("##Length", &(m_AggGeo.CapsuleElems[i].Length), 0.3f, 0, 0, "%.3f");

						ImGui::TreePop();
					}

					ImGui::PopID();
				}

				ImGui::TreePop();
			}

			ImGui::EndTable();
		}
	}

#endif

	void TriMeshGeom::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> CookData;
			PxDefaultMemoryInputData ReadStream(CookData.data(), CookData.size());
			TriMesh = PhysicManager::Get()->GetPhysics()->createTriangleMesh(ReadStream);
		}
		else
		{
			Ar << CookData;
		}
	}

}