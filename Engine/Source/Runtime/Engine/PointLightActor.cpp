#include "DrnPCH.h"
#include "PointLightActor.h"
#include "Runtime/Components/PointLightComponent.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
	REGISTER_SERIALIZABLE_ACTOR( EActorType::PointLight, PointLightActor );
	DECLARE_LEVEL_SPAWNABLE_CLASS( PointLightActor, Light );

	PointLightActor::PointLightActor()
		: LightActor()
	{
		m_PointLightComponent = std::make_unique<class PointLightComponent>();
		m_PointLightComponent->SetComponentLabel( "LightComponent" );
		SetRootComponent(m_PointLightComponent.get());
		SetLightComponent(m_PointLightComponent.get());

	}

	PointLightActor::~PointLightActor()
	{
		
	}

}