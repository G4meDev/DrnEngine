#include "DrnPCH.h"
#include "PrimitiveComponent.h"

namespace Drn
{
	PrimitiveComponent::PrimitiveComponent()
		: SceneComponent()
	{
		
	}

	PrimitiveComponent::~PrimitiveComponent()
	{
		
	}

	void PrimitiveComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

	}

	void PrimitiveComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

		m_BodyInstance.InitBody(nullptr, this, GetWorld()->GetPhysicScene());
	}

	void PrimitiveComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();

		m_BodyInstance.TermBody();
	}

}