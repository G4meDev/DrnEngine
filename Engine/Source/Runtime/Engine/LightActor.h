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
		inline void SetLightColor(const Vector& InColor) 
		{
			if (m_LightComponent)
			{
				m_LightComponent->SetColor( InColor );
			}
		}

		inline float GetIntensity() const { return m_LightComponent ? m_LightComponent->GetIntensity() : 1.0f; }
		inline void SetIntensity(float InIntensity) 
		{
			if (m_LightComponent)
			{
				m_LightComponent->SetIntensity( InIntensity);
			}
		}

		inline bool IsCastingShadow() const { return m_LightComponent ? m_LightComponent->IsCastingShadow() : false; }
		inline void SetCastShadow(bool InCastShadow) 
		{
			if (m_LightComponent)
			{
				m_LightComponent->SetCastShadow( InCastShadow );
			}
		}


		void SetLightComponent( LightComponent* InLightComponent ) { m_LightComponent = InLightComponent; }

	protected:
		LightActor();
		virtual ~LightActor();

		class LightComponent* m_LightComponent;

	private:

	};
}