#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class LightComponent : public SceneComponent
	{
	public:

		inline const Vector& GetLightColor() const { return m_LightColor; }
		inline float GetIntensity() const { return m_Intensity; }
		inline bool IsCastingShadow() const { return m_CastShadow; }

	protected:
		LightComponent();
		virtual ~LightComponent();

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		Vector m_LightColor;
		float m_Intensity;
		bool m_CastShadow;


	private:
	};
}