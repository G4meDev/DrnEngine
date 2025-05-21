#pragma once

#include "ForwardTypes.h"
#include "LightSceneProxy.h"

namespace Drn
{
	class PointLightSceneProxy : public LightSceneProxy
	{
	public:
		PointLightSceneProxy( class PointLightComponent* InComponent );
		virtual ~PointLightSceneProxy();

	protected:
		float m_Radius;

	private:

	};
}