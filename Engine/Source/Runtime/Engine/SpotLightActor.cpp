#include "DrnPCH.h"
#include "SpotLightActor.h"

#include "Runtime/Components/SpotLightComponent.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
	REGISTER_SERIALIZABLE_ACTOR( EActorType::SpotLight, SpotLightActor );
	DECLARE_LEVEL_SPAWNABLE_CLASS( SpotLightActor, Light );

	SpotLightActor::SpotLightActor()
		: LightActor()
	{
		m_SpotLightComponent = std::make_unique<class SpotLightComponent>();
		m_SpotLightComponent->SetComponentLabel( "LightComponent" );
		SetRootComponent(m_SpotLightComponent.get());
		SetLightComponent(m_SpotLightComponent.get());
	}

	SpotLightActor::~SpotLightActor()
	{
		
	}

}