#include "PointLightActor.h"
#include "DrnPCH.h"
#include "Runtime/Components/PointLightComponent.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
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

	void PointLightActor::Serialize( Archive& Ar )
	{
		LightActor::Serialize(Ar);

		m_PointLightComponent->Serialize(Ar);
	}

#if WITH_EDITOR
	void PointLightActor::DrawEditorSelected()
	{
		LightActor::DrawEditorSelected();

	}
#endif
}