#include "DrnPCH.h"
#include "SpotLightActor.h"

#include "Runtime/Components/SpotLightComponent.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
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

	void SpotLightActor::Serialize( Archive& Ar )
	{
		LightActor::Serialize(Ar);

		m_SpotLightComponent->Serialize(Ar);
	}

#if WITH_EDITOR
	void SpotLightActor::DrawEditorSelected()
	{
		LightActor::DrawEditorSelected();

	}
#endif

}