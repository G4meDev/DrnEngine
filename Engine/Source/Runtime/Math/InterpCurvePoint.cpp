#include "DrnPCH.h"
#include "InterpCurvePoint.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
#if WITH_EDITOR
	template<>
	bool InterpCurvePoint<float>::Draw( int32 Index )
	{
		bool bDirty = false;
		ImGui::PushID(Index);
		ImGui::Text(std::to_string(Index).c_str());

		bDirty |= ImGui::InputFloat("Time", &InVal);
		bDirty |= ImGui::InputFloat("Value", &OutVal);
		bDirty |= ImGui::InputFloat("Arrive Tangent", &ArriveTangent);
		bDirty |= ImGui::InputFloat("Leave Tangent", &LeaveTangent);

		const char* const Options[] = { "Linear", "CurveAuto", "Constant", "CurveUser", "CurveBreak", "CurveAutoClamped" };
		int32 Selected = (uint32)InterpMode;
		bDirty |= ImGui::Combo("Interp Mode", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			InterpMode = (EInterpCurveMode)Selected;
		}

		ImGui::Separator();

		ImGui::PopID();
		return bDirty;
	}

	template<>
	bool InterpCurvePoint<Vector>::Draw( int32 Index )
	{
		bool bDirty = false;
		ImGui::PushID(Index);
		ImGui::Text(std::to_string(Index).c_str());

		bDirty |= ImGui::InputFloat("Time", &InVal);
		bDirty |= OutVal.Draw("Value", "Value");
		bDirty |= ArriveTangent.Draw("Arrive Tangent", "Arrive Tangent");
		bDirty |= LeaveTangent.Draw("Leave Tangent", "Leave Tangent");

		const char* const Options[] = { "Linear", "CurveAuto", "Constant", "CurveUser", "CurveBreak", "CurveAutoClamped" };
		int32 Selected = (uint32)InterpMode;
		bDirty |= ImGui::Combo("Interp Mode", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			InterpMode = (EInterpCurveMode)Selected;
		}

		ImGui::Separator();

		ImGui::PopID();
		return bDirty;
	}
#endif
}