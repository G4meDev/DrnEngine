#include "DrnPCH.h"
#include "DecalActor.h"
#include "Runtime/Components/DecalComponent.h"

namespace Drn
{
	DecalActor::DecalActor()
		: Actor()
	{
		m_DecalComponent = std::make_unique<DecalComponent>();
		SetRootComponent(m_DecalComponent.get());
	}

	DecalActor::~DecalActor()
	{
		
	}

	void DecalActor::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);
	}
}