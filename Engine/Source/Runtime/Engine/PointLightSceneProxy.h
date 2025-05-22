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

		inline void SetRadius( float Radius ) { m_Radius = Radius; }

	protected:
		float m_Radius;


#if WITH_EDITOR
		virtual void DrawAttenuation(World* InWorld) override;
#endif

	private:

	};
}