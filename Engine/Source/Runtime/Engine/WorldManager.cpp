#include "DrnPCH.h"
#include "WorldManager.h"

namespace Drn
{
	WorldManager* WorldManager::m_SingletonInstance;

	WorldManager::WorldManager()
		: m_PendingLevel(nullptr)
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
		if (m_PendingLevel)
		{
			RemoveAndInvalidateWorld(m_MainWorld);
			m_MainWorld = m_PendingLevel;
			m_PendingLevel = nullptr;

			// TODO: move to renderer
			Renderer::Get()->m_MainScene = Renderer::Get()->AllocateScene(m_MainWorld);

			OnLevelChanged();
		}

		for (auto it = m_AllocatedWorlds.begin(); it != m_AllocatedWorlds.end(); it++)
		{
			(*it)->Tick(DeltaTime);
		}
	}

	void WorldManager::LoadInitalWorld()
	{
		m_SingletonInstance->LoadDefaultWorld();
	}

	void WorldManager::LoadLevel( const std::string& LevelPath )
	{
		AssetHandle<Level> NewLevel(LevelPath);
		NewLevel.Load();

		m_PendingLevel = AllocateWorld();
		NewLevel->LoadToWorld(m_PendingLevel);
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
		m_MainWorld->SetTransient(true);

		AssetHandle<StaticMesh> CubeMesh("Engine\\Content\\BasicShapes\\SM_Cube.drn");
		CubeMesh.Load();

		StaticMeshActor* CubeStaticMeshActor = m_MainWorld->SpawnActor<StaticMeshActor>();
		CubeStaticMeshActor->GetMeshComponent()->SetMesh(CubeMesh);

#if WITH_EDITOR
		CubeStaticMeshActor->SetActorLabel("Cube_1");
#endif
	}
}