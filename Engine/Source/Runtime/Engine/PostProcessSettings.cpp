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
		//if (Ar.IsLoading())
		//{
		//}
		//else
		//{
		//}

		m_SSAOSettings.Serialize(Ar);
	}

#if WITH_EDITOR
	void PostProcessSettings::Draw()
	{
		m_SSAOSettings.Draw();
	}
	void SSAOSettings::Draw()
	{
		ImGui::CollapsingHeader("AmbientOcclusion");
		ImGui::InputFloat("Intensity", &m_Intensity);
	}
#endif

}