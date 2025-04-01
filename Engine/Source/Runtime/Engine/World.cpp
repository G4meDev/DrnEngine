#include "DrnPCH.h"
#include "World.h"

namespace Drn
{
	World::World() 
		: m_ShouldTick(true)
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
		if (!m_ShouldTick)
		{
			return;
		}

		for (Actor* actor : m_Actors)
		{
			actor->Tick(DeltaTime);
		}
	}


}