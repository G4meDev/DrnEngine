#include "DrnPCH.h"
#include "LightActor.h"

namespace Drn
{
	LightActor::LightActor()
	{
		
	}

	LightActor::~LightActor()
	{
		
	}

	void LightActor::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

	}
}