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
	}

	void WorldManager::Shutdown()
	{
		for (World* W : m_SingletonInstance->m_AllocatedWorlds)
		{
			delete W;
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

}