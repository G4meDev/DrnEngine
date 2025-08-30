#pragma once

#include "ForwardTypes.h"
#include "LightActor.h"

namespace Drn
{
	class SpotLightActor : public LightActor
	{
	public:
		SpotLightActor();
		virtual ~SpotLightActor();

		virtual EActorType GetActorType() override { return EActorType::SpotLight; }
		inline static EActorType GetActorTypeStatic() { return EActorType::SpotLight; };

	protected:
		std::unique_ptr<class SpotLightComponent> m_SpotLightComponent;
	
	private:
	};
}