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
		m_SingletonInstance->OnCloseLevel.Braodcast( m_SingletonInstance->m_MainWorld );

		for (World* W : m_SingletonInstance->m_AllocatedWorlds)
		{
			W->DestroyInternal();
		}

		delete m_SingletonInstance;
		m_SingletonInstance = nullptr;
	}

	void WorldManager::Tick( float DeltaTime )
	{
		SCOPE_STAT();

		{
			SCOPE_STAT("PendingLevelCheck");

			if (m_PendingLevel)
			{
				OnCloseLevel.Braodcast( m_MainWorld );

				ReleaseWorld(m_MainWorld);
				m_MainWorld = m_PendingLevel;
				m_PendingLevel = nullptr;

				OnOpenLevel.Braodcast( m_MainWorld );
			}
		}

		{
			SCOPE_STAT("DestroyAndTickBroadcast");

			for (auto it = m_AllocatedWorlds.begin(); it != m_AllocatedWorlds.end();)
			{
				World* W = *it;

				if (W->IsPendingDestroy())
				{
					it = m_AllocatedWorlds.erase(it);
					ReleaseWorld(W);
				}

				else
				{
					W->Tick(DeltaTime);
					it++;
				}
			}
		}

	}

	void WorldManager::LoadInitalWorld()
	{
#if WITH_EDITOR
		m_SingletonInstance->LoadDefaultWorld();

		m_MainWorld->SetPaused(true);
		m_MainWorld->SetEditorWorld();
#else
		//m_SingletonInstance->LoadDefaultWorld();

		//AssetHandle<Level> DefaultLevel("Game\\Content\\Level_04.drn");
		AssetHandle<Level> DefaultLevel("Game\\Content\\Level_1\\Level_01.drn");
		DefaultLevel.Load();

		m_MainWorld = AllocateWorld();

		DefaultLevel->LoadToWorld(m_MainWorld);
		m_MainWorld->SetPaused(false);
		m_MainWorld->SetGameWorld();
		m_MainWorld->InitPlay();
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

			AssetHandle<StaticMesh> CubeMesh("Engine\\Content\\BasicShapes\\SM_Sphere.drn");
			CubeMesh.Load();

			StaticMeshActor* CubeStaticMeshActor = m_PendingLevel->SpawnActor<StaticMeshActor>();
			CubeStaticMeshActor->GetMeshComponent()->SetMesh(CubeMesh);
			CubeStaticMeshActor->SetActorLabel("Cube_1");
		}

#if WITH_EDITOR
		m_PendingLevel->SetPaused(true);
		m_PendingLevel->SetEditorWorld();
#else
		m_PendingLevel->SetPaused(false);
		m_PendingLevel->SetGameWorld();
#endif
	}

#if WITH_EDITOR

	void WorldManager::StartPlayInEditor()
	{
		if (m_MainWorld && !m_MainWorld->IsPlayInEditorWorld())
		{
			if (!m_MainWorld->IsTransient())
			{
				m_MainWorld->Save();
			}

			LoadLevel(m_LastLoadedLevel);
			m_PendingLevel->SetTransient(true);
			m_PendingLevel->SetPaused(false);
			m_PendingLevel->SetPlayInEditorWorld();
			m_PendingLevel->InitPlay();
		}
	}

	void WorldManager::EndPlayInEditor()
	{
		if ( m_MainWorld && m_MainWorld->IsPlayInEditorWorld() )
		{
			LoadLevel(m_LastLoadedLevel);
			m_PendingLevel->SetTransient(false);
			m_PendingLevel->SetPaused(true);
			m_PendingLevel->SetEditorWorld();
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
			InWorld->DestroyInternal();
			InWorld = nullptr;
		}
	}

	void WorldManager::LoadDefaultWorld()
	{
		m_MainWorld = AllocateWorld();
		m_MainWorld->SetTransient(true);

		AssetHandle<StaticMesh> CubeMesh("Engine\\Content\\BasicShapes\\SM_Sphere.drn");
		CubeMesh.Load();

		StaticMeshActor* CubeStaticMeshActor = m_MainWorld->SpawnActor<StaticMeshActor>();
		CubeStaticMeshActor->GetMeshComponent()->SetMesh(CubeMesh);

		PointLightActor* PointLight = m_MainWorld->SpawnActor<PointLightActor>();
		PointLight->SetActorLocation( Vector(2, 5, 3) );
		PointLight->SetRadius(10);

#if WITH_EDITOR
		CubeStaticMeshActor->SetActorLabel("Cube_1");
#endif
	}

}