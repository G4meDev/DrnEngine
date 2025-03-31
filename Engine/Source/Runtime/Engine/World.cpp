#include "DrnPCH.h"
#include "World.h"

namespace Drn
{
	World::World() 
	{
		
	}

	World::~World()
	{
		for (Actor* actor : m_Actors)
		{
			delete actor;
		}
	}

	void World::Tick( float DeltaTime )
	{
		for (Actor* actor : m_Actors)
		{
			actor->Tick(DeltaTime);
		}
	}


}