#include "DrnPCH.h"
#include "World.h"

#include "Runtime/Physic/PhysicManager.h"
#include "Runtime/Components/LineBatchComponent.h"

LOG_DEFINE_CATEGORY( LogWorld, "LogWorld" );

namespace Drn
{
	World::World() 
		: m_ShouldTick(false)
		, m_LevelPath("")
		, m_Transient(false)
	{
		m_PhysicScene = PhysicManager::Get()->AllocateScene(this);
		m_Scene = Renderer::Get()->AllocateScene(this);

		// TODO: proper camera and scene render managment
#ifndef WITH_EDITOR
		CameraActor* Cam = SpawnActor<CameraActor>();
		Cam->SetActorLocation(XMVectorSet(0, 0, -10, 0));

		Renderer::Get()->m_MainSceneRenderer = m_Scene->AllocateSceneRenderer();
		Renderer::Get()->m_MainSceneRenderer->m_CameraActor = Cam;		
#endif

		m_LineBatchCompponent = new LineBatchComponent();
		m_LineBatchCompponent->RegisterComponent(this);
	}

	World::~World()
	{
		
	}

	void World::Destroy()
	{
		m_LineBatchCompponent->UnRegisterComponent();
		delete m_LineBatchCompponent;
		m_LineBatchCompponent = nullptr;

		DestroyWorldActors();

		PhysicManager::Get()->RemoveAndInvalidateScene(m_PhysicScene);
		Renderer::Get()->ReleaseScene(m_Scene);

		delete this;
	}

	void World::Tick( float DeltaTime )
	{
		for (auto it = m_Actors.begin(); it != m_Actors.end();)
		{
			if (*it && (*it)->IsMarkedPendingKill())
			{
				Actor* ToDelActor = *it;
				InvokeOnRemoveActor(ToDelActor);
				ToDelActor->UnRegisterComponents();

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
				ToDelActor->UnRegisterComponents();

				it = m_NewActors.erase(it);
				delete ToDelActor;
			}
			else
			{
				(*it)->RegisterComponents(this);

				it++;
			}
		}

		m_Actors.insert(m_NewActors.begin(), m_NewActors.end());

		InvokeOnNewActors(m_NewActors);
		m_NewActors.clear();

		m_LineBatchCompponent->TickComponent(DeltaTime);

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

	void World::DrawDebugLine( const Vector& Start, const Vector& End, const Vector& Color, float Duration )
	{
		if (m_LineBatchCompponent)
		{
			m_LineBatchCompponent->DrawLine(Start, End, Color, Duration);
		}
	}

	void World::DrawDebugCircle( const Vector& Base, const Vector& X, const Vector& Z, const Vector& Color, float Radius, int32 NumSides, float Lifetime )
	{
		if (m_LineBatchCompponent)
		{
			m_LineBatchCompponent->DrawCircle(Base, X, Z, Color, Radius, NumSides, Lifetime);
		}
	}

	void World::DrawDebugSphere( const Vector& Center, const Quat& Rotation, const Vector& Color, float Radius, int32 NumSides, float Lifetime )
	{
		if (m_LineBatchCompponent)
		{
			m_LineBatchCompponent->DrawSphere(Center, Rotation, Color, Radius, NumSides, Lifetime);
		}
	}

	void World::DrawDebugBox( const Box& InBox, const Transform& T, const Vector& Color, float Lifetime )
	{
		if (m_LineBatchCompponent)
		{
			m_LineBatchCompponent->DrawBox(InBox, T, Color, Lifetime);
		}
	}

	void World::DestroyWorldActors()
	{
		for (auto it = m_Actors.begin(); it != m_Actors.end(); )
		{
			Actor* ToDelActor = *it;
			InvokeOnRemoveActor(ToDelActor);
			ToDelActor->UnRegisterComponents();
			it = m_Actors.erase(it);

			delete ToDelActor;
		}
		
		for (auto it = m_NewActors.begin(); it != m_NewActors.end(); )
		{
			Actor* ToDelActor = *it;
			InvokeOnRemoveActor(ToDelActor);
			ToDelActor->UnRegisterComponents();
			it = m_NewActors.erase(it);

			delete ToDelActor;
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