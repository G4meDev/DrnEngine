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
			Ar >> m_Power;
			Ar >> m_Bias;
			Ar >> m_Radius;
			Ar >> m_MipBlend;
			Ar >> m_FadeDistance;
			Ar >> m_FadeRadius;
		}
		else
		{
			Ar << m_Intensity;
			Ar << m_Power;
			Ar << m_Bias;
			Ar << m_Radius;
			Ar << m_MipBlend;
			Ar << m_FadeDistance;
			Ar << m_FadeRadius;
		}
	}

	void PostProcessSettings::Serialize( Archive& Ar )
	{
		m_SSAOSettings.Serialize(Ar);
		m_SSRSettings.Serialize(Ar);
	}

#if WITH_EDITOR
	bool PostProcessSettings::Draw()
	{
		bool Dirty = false;
		Dirty |= m_SSAOSettings.Draw();
		Dirty |= m_SSRSettings.Draw();
		return Dirty;
	}
	bool SSAOSettings::Draw()
	{
		bool Dirty = false;

		ImGui::PushID( "SSAO" );

		if (ImGui::CollapsingHeader("AmbientOcclusion", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			Dirty |= ImGui::DragFloat("Intensity", &m_Intensity, 0.1f, 0, 5, "%.1f");
			Dirty |= ImGui::DragFloat("Power", &m_Power, 0.1f, 0, 8, "%.1f");
			Dirty |= ImGui::DragFloat("Bias", &m_Bias, 0.00003f, 0, 0.1f, "%.5f");
			Dirty |= ImGui::DragFloat("Radius", &m_Radius, 0.01f, 0, 5, "%.2f");
			Dirty |= ImGui::DragFloat("MipBlend", &m_MipBlend, 0.05f, 0, 1, "%.2f");
			Dirty |= ImGui::DragFloat("FadeDistance", &m_FadeDistance, 0.1f, 0, 50, "%.1f");
			Dirty |= ImGui::DragFloat("FadeRadius", &m_FadeRadius, 0.1f, 0, 50, "%.1f");
		}

		ImGui::PopID();

		return Dirty;
	}
#endif

	void SSRSettings::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> m_Intensity;
			Ar >> m_RoughnessFade;
		}
		else
		{
			Ar << m_Intensity;
			Ar << m_RoughnessFade;
		}
	}

#if WITH_EDITOR
	bool SSRSettings::Draw()
	{
		bool Dirty = false;

		ImGui::PushID( "SSR" );

		if (ImGui::CollapsingHeader("Screen Space Reflection", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			Dirty |= ImGui::DragFloat("Intensity", &m_Intensity, 0.05f, 0, 1, "%.2f");
			Dirty |= ImGui::DragFloat("Roughness Fade", &m_RoughnessFade, 0.1f, -10.0f, -2.0f, "%.1f");
		}

		ImGui::PopID();

		return Dirty;
	}

#endif

}