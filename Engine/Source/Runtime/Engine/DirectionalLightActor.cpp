#include "DrnPCH.h"
#include "DirectionalLightActor.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
	REGISTER_SERIALIZABLE_ACTOR( EActorType::DirectionalLight, DirectionalLightActor );
	DECLARE_LEVEL_SPAWNABLE_CLASS( DirectionalLightActor, Light );

	DirectionalLightActor::DirectionalLightActor()
		: LightActor()
	{
		m_DirectionalLightComponent = std::make_unique<class DirectionalLightComponent>();
		m_DirectionalLightComponent->SetComponentLabel( "LightComponent" );
		SetRootComponent(m_DirectionalLightComponent.get());
		SetLightComponent(m_DirectionalLightComponent.get());
	}

	DirectionalLightActor::~DirectionalLightActor()
	{
		
	}

}