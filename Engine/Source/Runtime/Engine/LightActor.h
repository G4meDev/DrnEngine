#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Components/LightComponent.h"

namespace Drn
{
	class LightActor : public Actor
	{
	public:

		inline const Vector& GetLightColor() const { return m_LightComponent ? m_LightComponent->GetLightColor() : Vector::OneVector; }
		inline void SetLightColor() 
		{
			
		}

		void SetLightComponent( LightComponent* InLightComponent ) { m_LightComponent = InLightComponent; }

	protected:
		LightActor();
		virtual ~LightActor();

		class LightComponent* m_LightComponent;

	private:

	};
}