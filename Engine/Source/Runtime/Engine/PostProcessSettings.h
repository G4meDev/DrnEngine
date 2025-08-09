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
		{};

		virtual void Serialize(Archive& Ar) override;

		float m_Intensity;

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