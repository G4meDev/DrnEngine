#include "DrnPCH.h"
#include "GizmoState.h"

#if WITH_EDITOR
namespace Drn
{
	void GizmoState::GetSnapValue( float* SnapValue ) const 
	{
		float Result = 0.0f;

		if (m_Space == EGizmoSpace::Translation)
		{
			Result = GetTransitionSnapValue(m_TransitionSnap);
		}

		else if (m_Space == EGizmoSpace::Rotation)
		{
			Result = GetRotationSnapValue(m_RotationSnap);
		}

		else if (m_Space == EGizmoSpace::Scale)
		{
			Result = GetScaleSnapValue(m_ScaleSnap);
		}

		SnapValue[0] = SnapValue[1] = SnapValue[2] = Result;
	}

	void GizmoState::Draw()
	{
		if (ImGui::RadioButton("Translate", m_Space == EGizmoSpace::Translation))
		{
			m_Space = EGizmoSpace::Translation;
		}

		ImGui::SameLine();
		ImGui::PushID("TranslationSnap");
		if (ImGui::BeginCombo("##", GetTransitionSnapName(m_TransitionSnap).c_str(), ImGuiComboFlags_::ImGuiComboFlags_WidthFitPreview))
		{
			for (uint8 i = 0; i < static_cast<uint8>(ETranslationSnap::MAX); i++)
			{
				ETranslationSnap Snap = static_cast<ETranslationSnap>(i);
				if (ImGui::Selectable(GetTransitionSnapName( Snap ).c_str()))
				{
					m_TransitionSnap = Snap;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();

		ImGui::SameLine(0, 16);
		if (ImGui::RadioButton("Rotate", m_Space == EGizmoSpace::Rotation))
		{
			m_Space = EGizmoSpace::Rotation;
		}

		ImGui::SameLine();
		ImGui::PushID("RotationSnap");
		if (ImGui::BeginCombo("##", GetRotationSnapName(m_RotationSnap).c_str(), ImGuiComboFlags_::ImGuiComboFlags_WidthFitPreview))
		{
			for (uint8 i = 0; i < static_cast<uint8>(ERotationSnap::MAX); i++)
			{
				ERotationSnap Snap = static_cast<ERotationSnap>(i);
				if (ImGui::Selectable(GetRotationSnapName( Snap ).c_str()))
				{
					m_RotationSnap = Snap;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();

		ImGui::SameLine(0, 16);
		if (ImGui::RadioButton("Scale", m_Space == EGizmoSpace::Scale))
		{
			m_Space = EGizmoSpace::Scale;
		}

		ImGui::SameLine();
		ImGui::PushID("ScaleSnap");
		if (ImGui::BeginCombo("##", GetScaleSnapName(m_ScaleSnap).c_str(), ImGuiComboFlags_::ImGuiComboFlags_WidthFitPreview))
		{
			for (uint8 i = 0; i < static_cast<uint8>(EScaleSnap::MAX); i++)
			{
				EScaleSnap Snap = static_cast<EScaleSnap>(i);
				if (ImGui::Selectable(GetScaleSnapName( Snap ).c_str()))
				{
					m_ScaleSnap = Snap;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();

		ImGui::SameLine(0, 64);
		if (ImGui::RadioButton("Local", m_Mode == EGizmoMode::Local))
		{
			m_Mode = EGizmoMode::Local;
		}

		ImGui::SameLine();
		if (ImGui::RadioButton("World", m_Mode == EGizmoMode::World))
		{
			m_Mode = EGizmoMode::World;
		}
	}

	void GizmoState::HandleInput()
	{
		if (ImGui::IsKeyPressed(ImGuiKey_W))		{ m_Space = EGizmoSpace::Translation; }
		else if (ImGui::IsKeyPressed(ImGuiKey_E))	{ m_Space = EGizmoSpace::Rotation; }
		else if (ImGui::IsKeyPressed(ImGuiKey_R))	{ m_Space = EGizmoSpace::Scale; }
		else if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) { m_Mode = m_Mode == EGizmoMode::Local ?
			EGizmoMode::World : EGizmoMode ::Local; }
	}

	std::string GizmoState::GetTransitionSnapName( ETranslationSnap Snap ) const
	{
		switch ( Snap )
		{
		case ETranslationSnap::NoSnap:	return ".";
		case ETranslationSnap::Snap_01:	return "0.1";
		case ETranslationSnap::Snap_05:	return "0.5";
		case ETranslationSnap::Snap_1:	return "1";
		case ETranslationSnap::Snap_10:	return "10";
		default:						return ".";
		}
	}

	std::string GizmoState::GetRotationSnapName( ERotationSnap Snap ) const
	{
		switch ( Snap )
		{
		case ERotationSnap::NoSnap:		return ".";
		case ERotationSnap::Snap_1:		return "1";
		case ERotationSnap::Snap_5:		return "5";
		case ERotationSnap::Snap_30:	return "30";
		case ERotationSnap::Snap_45:	return "45";
		default:						return ".";
		}
	}

	std::string GizmoState::GetScaleSnapName( EScaleSnap Snap ) const
	{
		switch ( Snap )
		{
		case EScaleSnap::NoSnap:		return ".";
		case EScaleSnap::Snap_025:		return "0.25";
		case EScaleSnap::Snap_1:		return "1";
		case EScaleSnap::Snap_5:		return "5";
		case EScaleSnap::Snap_10:		return "10";
		default:						return ".";
		}
	}

	float GizmoState::GetTransitionSnapValue( ETranslationSnap Snap ) const
	{
		switch ( Snap )
		{
		case ETranslationSnap::NoSnap:	return 0.0f;
		case ETranslationSnap::Snap_01:	return 0.1f;
		case ETranslationSnap::Snap_05:	return 0.5f;
		case ETranslationSnap::Snap_1:	return 1.0f;
		case ETranslationSnap::Snap_10:	return 10.0f;
		default:						return 0.0f;
		}
	}

	float GizmoState::GetRotationSnapValue( ERotationSnap Snap ) const
	{
		switch ( Snap )
		{
		case ERotationSnap::NoSnap:		return 0.0f;
		case ERotationSnap::Snap_1:		return 1.0f;
		case ERotationSnap::Snap_5:		return 5.0f;
		case ERotationSnap::Snap_30:	return 30.0f;
		case ERotationSnap::Snap_45:	return 45.0f;
		default:						return 0.0f;
		}
	}

	float GizmoState::GetScaleSnapValue( EScaleSnap Snap ) const
	{
		switch ( Snap )
		{
		case EScaleSnap::NoSnap:		return 0.0f;
		case EScaleSnap::Snap_025:		return 0.25f;
		case EScaleSnap::Snap_1:		return 1.0f;
		case EScaleSnap::Snap_5:		return 5.0f;
		case EScaleSnap::Snap_10:		return 10.0f;
		default:						return 0.0f;
		}
	}

}
#endif