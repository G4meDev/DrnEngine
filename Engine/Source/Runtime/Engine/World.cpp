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

		for (CameraComponent* Camera : m_CameraComponents)
		{
			delete Camera;
		}

		m_StaticMeshComponents.clear();
	}

	void World::Tick( float DeltaTime )
	{
		
	}

	void World::AddStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		m_StaticMeshComponents.push_back(InStaticMesh);
	}

	void World::RemoveStaticMeshCompponent( StaticMeshComponent* InStaticMesh )
	{
		
	}

	void World::AddCameraComponent( CameraComponent* InCamera )
	{
		m_CameraComponents.push_back(InCamera);
	}

	void World::RemoveCameraComponent( CameraComponent* InCamera )
	{
		
	}

}