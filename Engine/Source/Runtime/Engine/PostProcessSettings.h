#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Archive.h"

namespace Drn
{
	class SSAOSettings : public Serializable
	{
	public:

		SSAOSettings()
			: m_Intensity(1.0f)
			, m_Power(4.0f)
			, m_Bias(0.0003f)
			, m_Radius(0.1f)
			, m_MipBlend(0.6f)
			, m_FadeDistance(5.0f)
			, m_FadeRadius(30.0f)
		{};

		virtual void Serialize(Archive& Ar) override;

		float m_Intensity;
		float m_Power;
		float m_Bias;
		float m_Radius;

		float m_MipBlend;
		float m_FadeDistance;
		float m_FadeRadius;

#if WITH_EDITOR
		bool Draw();
#endif
	};

	class PostProcessSettings : public Serializable
	{
	public:

		PostProcessSettings() = default;
		virtual void Serialize(Archive& Ar) override;

		static PostProcessSettings DefaultSettings;

		SSAOSettings m_SSAOSettings;

#if WITH_EDITOR
		bool Draw();
#endif
	};
}