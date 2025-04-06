#include "DrnPCH.h"
#include "World.h"

LOG_DEFINE_CATEGORY( LogWorld, "LogWorld" );

namespace Drn
{
	World::World() 
		: m_ShouldTick(false)
		, m_LevelPath("")
		, m_Transient(false)
	{
		
	}

	World::~World()
	{
		for (Actor* actor : m_Actors)
		{
			delete actor;
		}

		if (Renderer::Get())
		{
			Renderer::Get()->RemoveWorldScenes(this);
		}

	}

	void World::Tick( float DeltaTime )
	{
		for (auto it = m_Actors.begin(); it != m_Actors.end();)
		{
			if (*it && (*it)->IsMarkedPendingKill())
			{
				Actor* ToDelActor = *it;
				InvokeOnRemoveActor(ToDelActor);
		
				it = m_Actors.erase(it);
				delete ToDelActor;
			}

			else
			{
				it++;
			}
		}
		
		for (auto it = m_NewActors.begin(); it != m_NewActors.end();)
		{
			if (*it && (*it)->IsMarkedPendingKill())
			{
				Actor* ToDelActor = *it;
				InvokeOnRemoveActor(ToDelActor);
		
				it = m_NewActors.erase(it);
				delete ToDelActor;
			}
			else
			{
				it++;
			}
		}

		m_Actors.insert(m_NewActors.begin(), m_NewActors.end());

		InvokeOnNewActors(m_NewActors);
		m_NewActors.clear();

		if (!m_ShouldTick)
		{
			return;
		}

		for (Actor* actor : m_Actors)
		{
			actor->Tick(DeltaTime);
		}
	}

// ----------------------------------------------------------------------------------------

	void World::BindOnNewActors( OnNewActors Delegate )
	{
		OnNewActorsDelegates.push_back(Delegate);
	}
	
	void World::RemoveFromOnNewActors( OnNewActors Delegate )
	{
		//OnNewActorsDelegates.erase(Delegate);
	}
	
	void World::InvokeOnNewActors( const std::set<Actor*>& NewActors )
	{
		for (const OnNewActors& Del : OnNewActorsDelegates)
		{
			Del(NewActors);
		}
	}

// ----------------------------------------------------------------------------------------

	void World::BindOnRemoveActor( OnRemoveActor Delegate )
	{
		OnRemoveActorDelegates.push_back(Delegate);
	}

	void World::RemoveFromOnRemoveActor( OnRemoveActor Delegate )
	{
		
	}

	void World::InvokeOnRemoveActor( const Actor* RemovedActor )
	{
		for (const OnRemoveActor& Del : OnRemoveActorDelegates)
		{
			Del(RemovedActor);
		}
	}

#if WITH_EDITOR

	void World::Save()
	{
		if (IsTransient())
		{
			LOG(LogWorld, Warning, "trying to save a transient world.");
			return;
		}

		AssetHandle<Level> LevelHandle(m_LevelPath);
		LevelHandle.Load();

		LevelHandle->SaveFromWorld(this);
	}

	uint32 World::GetNonTransientActorCount()
	{
		int result = 0;

		for (Actor* Actor : m_Actors)
		{
			if (!Actor->IsTransient())
			{
				result++;
			}
		}

		return result;
	}

#endif
}