#include "DrnPCH.h"
#include "Component.h"

namespace Drn
{
	Component::Component()
	{
		
	}

	void Component::Tick(float DeltaTime)
	{
		
	}

	Actor* Component::GetOwningActor() const
	{
		return Owner;
	}

	void Component::SetOwningActor(Actor* InActor)
	{
		Owner = InActor;
	}

	bool Component::IsActive() const
	{
		return bActive;
	}

	void Component::SetActive(bool Active)
	{
		bActive = Active;
	}

#if WITH_EDITOR
	std::string Component::GetComponentLabel() const
	{
		return ComponentLabel;
	}

	void Component::SetComponentLabel( const std::string& InLabel )
	{
		ComponentLabel = InLabel;
	}

#endif
}