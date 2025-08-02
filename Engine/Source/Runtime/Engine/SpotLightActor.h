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
	
		//inline float GetRadius() const
		//{
		//	return m_PointLightComponent ? m_PointLightComponent->GetRadius() : 1.0f;
		//}
		//inline void SetRadius( float InRadius )
		//{
		//	if ( m_PointLightComponent )
		//	{
		//		m_PointLightComponent->SetRadius( InRadius );
		//	}
		//}
	
	protected:
		virtual EActorType GetActorType() override
		{
			return EActorType::SpotLight;
		}
		std::unique_ptr<class SpotLightComponent> m_SpotLightComponent;
	
	private:
	};
}