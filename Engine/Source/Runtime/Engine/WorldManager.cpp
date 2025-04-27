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
		for (World* W : m_SingletonInstance->m_AllocatedWorlds)
		{
			W->Destroy();
		}

		delete m_SingletonInstance;
		m_SingletonInstance = nullptr;
	}

	void WorldManager::Tick( float DeltaTime )
	{
		SCOPE_STAT(WorldManagerTick);

		if (m_PendingLevel)
		{
			ReleaseWorld(m_MainWorld);
			m_MainWorld = m_PendingLevel;
			m_PendingLevel = nullptr;

			OnLevelChanged();
		}

		for (auto it = m_AllocatedWorlds.begin(); it != m_AllocatedWorlds.end(); it++)
		{
			(*it)->Tick(DeltaTime);
		}
	}

	void WorldManager::LoadInitalWorld()
	{
#if WITH_EDITOR
		m_SingletonInstance->LoadDefaultWorld();
#else
		//m_SingletonInstance->LoadDefaultWorld();

		AssetHandle<Level> DefaultLevel("Game\\Content\\Level_01.drn");
		DefaultLevel.Load();

		m_MainWorld = AllocateWorld();
		m_MainWorld->SetTickEnabled(true);

		DefaultLevel->LoadToWorld(m_MainWorld);
#endif
	}

	void WorldManager::LoadLevel( const std::string& LevelPath )
	{
		m_LastLoadedLevel = LevelPath;

		if (LevelPath != "")
		{
			AssetHandle<Level> NewLevel(LevelPath);
			NewLevel.Load();

			m_PendingLevel = AllocateWorld();
			NewLevel->LoadToWorld(m_PendingLevel);
		}

		else
		{
			m_PendingLevel = AllocateWorld();
			m_PendingLevel->SetTransient(true);

			AssetHandle<StaticMesh> CubeMesh("Engine\\Content\\BasicShapes\\SM_Cube.drn");
			CubeMesh.Load();

			StaticMeshActor* CubeStaticMeshActor = m_PendingLevel->SpawnActor<StaticMeshActor>();
			CubeStaticMeshActor->GetMeshComponent()->SetMesh(CubeMesh);

#if WITH_EDITOR
			CubeStaticMeshActor->SetActorLabel("Cube_1");
#endif
		}
	}

#if WITH_EDITOR

	void WorldManager::StartPlayInEditor()
	{
		if (m_PlayInEditor)
		{
			return;
		}

		m_PlayInEditor = true;
		m_PlayInEditorPaused = false;

		if (!m_MainWorld->IsTransient())
		{
			m_MainWorld->Save();
		}

		LoadLevel(m_LastLoadedLevel);
		m_PendingLevel->SetTransient(true);
		m_PendingLevel->m_ShouldTick = true;
	}

	void WorldManager::EndPlayInEditor()
	{
		if (!m_PlayInEditor)
		{
			return;
		}

		m_PlayInEditor = false;

		LoadLevel(m_LastLoadedLevel);
	}

	void WorldManager::SetPlayInEditorPaused( bool Paused )
	{
		if (m_PlayInEditor)
		{
			m_PlayInEditorPaused = Paused;

			m_MainWorld->SetTickEnabled(!m_PlayInEditorPaused);
		}
	}

#endif

	World* WorldManager::AllocateWorld()
	{
		World* NewWorld = new World();
		m_AllocatedWorlds.insert(NewWorld);

		return NewWorld;
	}

	void WorldManager::ReleaseWorld( World*& InWorld )
	{
		m_AllocatedWorlds.erase(InWorld);

		if (InWorld)
		{
			InWorld->Destroy();
			InWorld = nullptr;
		}
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