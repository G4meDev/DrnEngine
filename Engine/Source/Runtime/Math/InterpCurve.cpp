#include "DrnPCH.h"
#include "InterpCurve.h"

namespace Drn
{
#if WITH_EDITOR
	template<>
	bool InterpCurve<float>::Draw(const std::string DisplayName)
	{
		bool bDirty = false;

		if (ImGui::CollapsingHeader(DisplayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayName.c_str());

			const int32 DisplayCurvePoints = 50;
			float DisplayCurveValues[DisplayCurvePoints];
			for (int32 i = 0; i < DisplayCurvePoints; i++)
			{
				float NormalizedTime = (float)i / (DisplayCurvePoints - 1);

				float Sample = Eval(NormalizedTime, 0.0);
				DisplayCurveValues[i] = Sample;
			}
			ImGui::PlotLines("Curve", DisplayCurveValues, DisplayCurvePoints);

			if (ImGui::Button("Add"))
			{
				AddPoint(1.0f, 0.0);
			}

			bDirty |= ImGui::Checkbox("Is Looped", &bIsLooped);
			bDirty |= ImGui::InputFloat("Loop Key Offset", &LoopKeyOffset);

			for (int i = 0; i < Points.size(); i++)
			{
				bDirty |= Points[i].Draw(i);
			}

			ImGui::PopID();
		}

		return bDirty;
	}

	template<>
	bool InterpCurve<Vector>::Draw(const std::string DisplayName)
	{
		bool bDirty = false;

		if (ImGui::CollapsingHeader(DisplayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayName.c_str());

			for (int32 Axis = 0; Axis < 3; Axis++)
			{
				ImGui::PushID(Axis);

				const int32 DisplayCurvePoints = 50;
				float DisplayCurveValues[DisplayCurvePoints];
				for (int32 i = 0; i < DisplayCurvePoints; i++)
				{
					float NormalizedTime = (float)i / (DisplayCurvePoints - 1);

					Vector Sample = Eval(NormalizedTime, 0.0);
					DisplayCurveValues[i] = Sample[Axis];
				}
				ImGui::PlotLines("Curve", DisplayCurveValues, DisplayCurvePoints);

				ImGui::PopID();
			}

			if (ImGui::Button("Add"))
			{
				AddPoint(1.0f, 0.0);
			}

			bDirty |= ImGui::Checkbox("Is Looped", &bIsLooped);
			bDirty |= ImGui::InputFloat("Loop Key Offset", &LoopKeyOffset);

			for (int i = 0; i < Points.size(); i++)
			{
				bDirty |= Points[i].Draw(i);
			}

			ImGui::PopID();
		}

		return bDirty;
	}
#endif
}