#include "DrnPCH.h"
#include "World.h"

namespace Drn
{
	World::World() 
	{
		
	}

	World::~World()
	{
		for (StaticMeshComponent* Mesh : m_StaticMeshComponents)
		{
			delete Mesh;
		}

		m_StaticMeshComponents.clear();
	}

	void World::AddStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		m_StaticMeshComponents.push_back(InStaticMesh);
	}

	void World::RemoveStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		
	}

}