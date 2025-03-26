#include "DrnPCH.h"
#include "World.h"

namespace Drn
{
	World::World() 
	{
		
	}

	World::~World()
	{
		
	}

	void World::AddPrimitive( PrimitiveComponent* InPrimitive )
	{
		m_PrimitiveComponents.push_back(InPrimitive);
	}

	void World::RemovePrimitive( PrimitiveComponent* InPrimitive )
	{
		
	}

}