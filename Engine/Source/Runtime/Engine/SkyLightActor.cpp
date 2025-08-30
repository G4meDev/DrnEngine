#include "DrnPCH.h"
#include "SkyLightActor.h"

#include "Runtime/Components/SkyLightComponent.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
	REGISTER_SERIALIZABLE_ACTOR( EActorType::SkyLight, SkyLightActor );
	DECLARE_LEVEL_SPAWNABLE_CLASS( SkyLightActor, Light );

	SkyLightActor::SkyLightActor()
		: LightActor()
	{
		m_SkyLightComponent = std::make_unique<class SkyLightComponent>();
		m_SkyLightComponent->SetComponentLabel( "LightComponent" );
		SetRootComponent(m_SkyLightComponent.get());
		SetLightComponent(m_SkyLightComponent.get());
	}

	SkyLightActor::~SkyLightActor()
	{
		
	}

}