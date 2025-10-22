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
			, m_Power(2.0f)
			, m_Bias(0.003f)
			, m_Radius(0.02f)
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

	class SSRSettings : public Serializable
	{
	public:

		SSRSettings()
			: m_Intensity(1.0f)
			, m_RoughnessFade(-4.0f)
		{};

		virtual void Serialize(Archive& Ar) override;

		float m_Intensity;
		float m_RoughnessFade;

#if WITH_EDITOR
		bool Draw();
#endif
	};

	class TAASettings : public Serializable
	{
	public:

		TAASettings()
			: m_JitterOffsetScale(0.8f)
			, m_CurrentFrameWeight(0.04f)
			, m_CcurrentFrameVelocityWeight(0.2f)
			, m_CcurrentFrameVelocityMultiplier(7.0f)
		{};

		virtual void Serialize(Archive& Ar) override;

		float m_JitterOffsetScale;
		float m_CurrentFrameWeight;
		float m_CcurrentFrameVelocityWeight;
		float m_CcurrentFrameVelocityMultiplier;

#if WITH_EDITOR
		bool Draw();
#endif
	};

	class BloomSettings : public Serializable
	{
	public:

		BloomSettings()
			: m_Radius(1.0f)
			, m_Brightness(0.2f)
			, m_Sigma(0.85f)
		{};

		virtual void Serialize(Archive& Ar) override;

		float m_Radius;
		float m_Brightness;
		float m_Sigma;

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
		SSRSettings m_SSRSettings;
		TAASettings m_TAASettings;
		BloomSettings m_BloomSettings;

#if WITH_EDITOR
		bool Draw();
#endif
	};
}