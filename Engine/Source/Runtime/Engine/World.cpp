#include "DrnPCH.h"
#include "World.h"

namespace Drn
{
	World::World() 
		: m_ShouldTick(true)
	{
		
	}

	World::~World()
	{
		for (Actor* actor : m_Actors)
		{
			delete actor;
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

}