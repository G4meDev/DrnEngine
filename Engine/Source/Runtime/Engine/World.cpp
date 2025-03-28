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

	void World::AddStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		m_StaticMeshComponents.push_back(InStaticMesh);
	}

	void World::RemoveStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		
	}

}