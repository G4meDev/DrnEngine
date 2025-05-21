#pragma once

#include "ForwardTypes.h"
#include "LightComponent.h"

namespace Drn
{
	class PointLightComponent : public LightComponent
	{
	public:
		PointLightComponent();
		virtual ~PointLightComponent();

		inline float GetRadius() const { return m_Radius; }

	protected:

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		float m_Radius;

		class PointLightSceneProxy* m_SceneProxy;

	private:
	};
}