#pragma once

#include "ForwardTypes.h"
#include "LightActor.h"
#include "Runtime/Components/PointLightComponent.h"

namespace Drn
{
	class PointLightActor : public LightActor
	{
	public:
		PointLightActor();
		virtual ~PointLightActor();

		inline float GetRadius() const { return m_PointLightComponent ? m_PointLightComponent->GetRadius() : 1.0f; }
		inline void SetRadius(float InRadius) 
		{
			if (m_PointLightComponent)
			{
				m_PointLightComponent->SetRadius( InRadius);
			}
		}

	protected:

		virtual EActorType GetActorType() override { return EActorType::PointLight; }
		std::unique_ptr<class PointLightComponent> m_PointLightComponent;

	private:
	};
}