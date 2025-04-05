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

}
