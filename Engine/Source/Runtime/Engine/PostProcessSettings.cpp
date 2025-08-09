#include "DrnPCH.h"
#include "PostProcessSettings.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	PostProcessSettings PostProcessSettings::DefaultSettings;


	void SSAOSettings::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> m_Intensity;
		}
		else
		{
			Ar << m_Intensity;
		}
	}

	void PostProcessSettings::Serialize( Archive& Ar )
	{
		m_SSAOSettings.Serialize(Ar);
	}

#if WITH_EDITOR
	bool PostProcessSettings::Draw()
	{
		bool Dirty = false;
		Dirty |= m_SSAOSettings.Draw();
		return Dirty;
	}
	bool SSAOSettings::Draw()
	{
		bool Dirty = false;

		if (ImGui::CollapsingHeader("AmbientOcclusion", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			Dirty |= ImGui::DragFloat("Intensity", &m_Intensity, 0.1f, 0, 5, "%.1f");
		}

		return Dirty;
	}
#endif

}