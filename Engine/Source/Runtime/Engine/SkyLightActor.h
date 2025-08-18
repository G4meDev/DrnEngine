#pragma once

#include "ForwardTypes.h"
#include "LightActor.h"

namespace Drn
{
	class SkyLightActor : public LightActor
	{
	public:
		SkyLightActor();
		virtual ~SkyLightActor();

	protected:
		virtual EActorType GetActorType() override
		{
			return EActorType::SkyLight;
		}
		std::unique_ptr<class SkyLightComponent> m_SkyLightComponent;
	
	private:
	};
}