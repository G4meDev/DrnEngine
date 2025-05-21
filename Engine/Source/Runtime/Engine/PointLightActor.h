#pragma once

#include "ForwardTypes.h"
#include "LightActor.h"

namespace Drn
{
	class PointLightActor : public LightActor
	{
	public:
		PointLightActor();
		virtual ~PointLightActor();

	protected:

		virtual EActorType GetActorType() override { return EActorType::PointLight; }
		std::unique_ptr<class PointLightComponent> m_PointLightComponent;

	private:
	};
}