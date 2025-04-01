#include "DrnPCH.h"
#include "WorldManager.h"

namespace Drn
{
	WorldManager* WorldManager::m_SingletonInstance;

	WorldManager::WorldManager()
	{
	}

	void WorldManager::Init()
	{
		m_SingletonInstance = new WorldManager();
		m_SingletonInstance->LoadInitalWorld();
	}

	void WorldManager::Shutdown()
	{
		if (m_SingletonInstance->m_MainWorld)
		{
			delete m_SingletonInstance->m_MainWorld;
			m_SingletonInstance->m_MainWorld = nullptr;
		}

		delete m_SingletonInstance;
		m_SingletonInstance = nullptr;
	}

	void WorldManager::Tick( float DeltaTime )
	{
		for (auto it = m_AllocatedWorlds.begin(); it != m_AllocatedWorlds.end(); it++)
		{
			(*it)->Tick(DeltaTime);
		}
	}

	void WorldManager::LoadInitalWorld()
	{
		m_SingletonInstance->LoadDefaultWorld();
	}

	World* WorldManager::AllocateWorld()
	{
		World* NewWorld = new World();
		m_AllocatedWorlds.insert(NewWorld);

		return NewWorld;
	}

	void WorldManager::RemoveWorld( World* InWorld )
	{
		m_AllocatedWorlds.erase(InWorld);
	}

	void WorldManager::RemoveAndInvalidateWorld( World*& InWorld )
	{
		RemoveWorld(InWorld);
		delete InWorld;
		InWorld = nullptr;
	}

	void WorldManager::LoadDefaultWorld()
	{
		m_MainWorld = AllocateWorld();

		AssetHandle<StaticMesh> CubeMesh(Path::ConvertFullPath("BasicShapes\\SM_Cube.drn"));
		CubeMesh.Load();

		StaticMeshActor* CubeStaticMeshActor = m_MainWorld->SpawnActor<StaticMeshActor>();
		CubeStaticMeshActor->GetMeshComponent()->SetMesh(CubeMesh);
	}
}