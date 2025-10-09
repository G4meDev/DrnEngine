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
		m_TAASettings.Serialize(Ar);
		m_BloomSettings.Serialize(Ar);
	}

#if WITH_EDITOR
	bool PostProcessSettings::Draw()
	{
		bool Dirty = false;
		Dirty |= m_SSAOSettings.Draw();
		Dirty |= m_SSRSettings.Draw();
		Dirty |= m_TAASettings.Draw();
		Dirty |= m_BloomSettings.Draw();
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

	void TAASettings::Serialize( Archive& Ar )
	{
		
	}

#if WITH_EDITOR
	bool TAASettings::Draw()
	{
		bool Dirty = false;

		ImGui::PushID( "TAA" );

		if (ImGui::CollapsingHeader("TemporalAA", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			Dirty |= ImGui::DragFloat("JitterOffsetScacle", &m_JitterOffsetScale, 0.05f, 0, 10, "%.2f");
			Dirty |= ImGui::DragFloat("CurrentFrameWeight", &m_CurrentFrameWeight, 0.05f, 0, 1, "%.2f");
			Dirty |= ImGui::DragFloat("CcurrentFrameVelocityWeight", &m_CcurrentFrameVelocityWeight, 0.05f, 0, 1, "%.2f");
			Dirty |= ImGui::DragFloat("CcurrentFrameVelocityMultiplier", &m_CcurrentFrameVelocityMultiplier, 0.05f, 0, 20, "%.2f");
		}

		ImGui::PopID();

		return Dirty;
	}
#endif

	void BloomSettings::Serialize( Archive& Ar )
	{
		
	}

#if WITH_EDITOR
	bool BloomSettings::Draw()
	{
		bool Dirty = false;

		ImGui::PushID( "Bloom" );

		if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			Dirty |= ImGui::DragFloat("Radius", &m_Radius, 0.05f, 0, 10, "%.2f");
			Dirty |= ImGui::DragFloat("Brightness", &m_Brightness, 0.05f, 0, 10, "%.2f");
			Dirty |= ImGui::DragFloat("Sigma", &m_Sigma, 0.05f, 0, 10, "%.2f");
		}

		ImGui::PopID();

		return Dirty;
	}
#endif

}  // namespace Drn