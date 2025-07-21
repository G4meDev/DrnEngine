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
		, m_PendingDestory(false)
		, m_TimeSeconds(0)
	{
		m_PhysicScene = PhysicManager::Get()->AllocateScene(this);
		m_Scene = Renderer::Get()->AllocateScene(this);

		// TODO: proper camera and scene render management
#ifndef WITH_EDITOR
		CameraActor* Cam = SpawnActor<CameraActor>();
		Cam->SetActorLocation(XMVectorSet(0, 0, -10, 0));

		Renderer::Get()->m_MainSceneRenderer = m_Scene->AllocateSceneRenderer();
		Renderer::Get()->m_MainSceneRenderer->m_CameraActor = Cam;
#endif

#if WITH_EDITOR

		AssetHandle<StaticMesh> AxisGridMesh( "Engine\\Content\\BasicShapes\\SM_Plane.drn" );
		AxisGridMesh.Load();
		AssetHandle<Material> AxisGridMaterial( "Engine\\Content\\Materials\\M_AxisGridMaterial.drn" );
		AxisGridMaterial.Load();
		
		m_AxisGridPlane = SpawnActor<StaticMeshActor>();
		m_AxisGridPlane->SetActorLabel("AxisGrid");
		m_AxisGridPlane->SetTransient(true);
		m_AxisGridPlane->GetMeshComponent()->SetSelectable(false);
		m_AxisGridPlane->SetActorScale(Vector( 1000.0f ));
		m_AxisGridPlane->SetActorLocation(Vector::ZeroVector);
		m_AxisGridPlane->GetMeshComponent()->SetMesh(AxisGridMesh);
		m_AxisGridPlane->GetMeshComponent()->SetMaterial(0, AxisGridMaterial);
		m_AxisGridPlane->GetMeshComponent()->SetEditorPrimitive(true);

#endif

		m_LineBatchCompponent = new LineBatchComponent();
		m_LineBatchCompponent->SetThickness(false);
		m_LineBatchCompponent->RegisterComponent(this);

		m_LineBatchThicknessCompponent = new class LineBatchComponent();
		m_LineBatchThicknessCompponent->SetThickness(true);
		m_LineBatchThicknessCompponent->RegisterComponent(this);
	}

	World::~World()
	{
		
	}

	void World::Destroy()
	{
		m_PendingDestory = true;
	}

	void World::Tick( float DeltaTime )
	{
		SCOPE_STAT();

		m_TimeSeconds += DeltaTime;

		{
			SCOPE_STAT("PendingDestroyAddActors");

			for (auto it = m_Actors.begin(); it != m_Actors.end();)
			{
				if (*it && (*it)->IsMarkedPendingKill())
				{
					Actor* ToDelActor = *it;

					std::vector<Actor*> RemovedActorList;
					RemovedActorList.push_back(ToDelActor);
					OnRemoveActors.Braodcast( RemovedActorList );

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

					std::vector<Actor*> RemovedActorList;
					RemovedActorList.push_back(ToDelActor);
					OnRemoveActors.Braodcast( RemovedActorList );

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

			if (m_NewActors.size())
			{
				m_Actors.insert(m_NewActors.begin(), m_NewActors.end());

				OnAddActors.Braodcast(m_NewActors);
				m_NewActors.clear();
			}
		}

		m_LineBatchCompponent->TickComponent(DeltaTime);
		m_LineBatchThicknessCompponent->TickComponent(DeltaTime);

		if (!m_ShouldTick)
		{
			return;
		}

		{
			SCOPE_STAT();

			for (Actor* actor : m_Actors)
			{
				actor->Tick(DeltaTime);
			}
		}
	}

	Component* World::GetComponentWithGuid( const Guid& ID )
	{
		if (ID.IsValid())
		{
			for (Actor* actor : m_Actors)
			{
				if (!actor->IsMarkedPendingKill())
				{
					std::vector<Component*> Components;
					actor->GetComponents<Component>(Components, EComponentType::Component, true);

					for (Component* Comp : Components)
					{
						if (Comp->GetGuid() == ID)
						{
							return Comp;
						}
					}
				}
			}
		}

		return nullptr;
	}

// ----------------------------------------------------------------------------------------

	void World::DrawDebugLine( const Vector& Start, const Vector& End, const Color& Color, float Thickness, float Duration )
	{
		if (m_LineBatchCompponent && Thickness == 0)
		{
			m_LineBatchCompponent->DrawLine(Start, End, Color, Thickness, Duration);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawLine(Start, End, Color, Thickness, Duration);
		}
	}

	void World::DrawDebugCircle( const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime )
	{
		if (m_LineBatchCompponent && Thickness == 0)
		{
			m_LineBatchCompponent->DrawCircle(Base, X, Z, Color, Radius, NumSides, Thickness, Lifetime);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawCircle(Base, X, Z, Color, Radius, NumSides, Thickness, Lifetime);
		}
	}

	void World::DrawDebugSphere( const Vector& Center, const Quat& Rotation, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime )
	{
		if (m_LineBatchCompponent && Thickness == 0)
		{
			m_LineBatchCompponent->DrawSphere(Center, Rotation, Color, Radius, NumSides, Thickness, Lifetime);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawSphere(Center, Rotation, Color, Radius, NumSides, Thickness, Lifetime);
		}
	}

	void World::DrawDebugBox( const Box& InBox, const Transform& T, const Color& Color, float Thickness, float Lifetime )
	{
		if (m_LineBatchCompponent)
		{
			m_LineBatchCompponent->DrawBox(InBox, T, Color, Thickness, Lifetime);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawBox(InBox, T, Color, Thickness, Lifetime);
		}
	}

	void World::DestroyInternal()
	{
		m_LineBatchCompponent->UnRegisterComponent();
		delete m_LineBatchCompponent;
		m_LineBatchCompponent = nullptr;

		DestroyWorldActors();

		PhysicManager::Get()->RemoveAndInvalidateScene(m_PhysicScene);
		Renderer::Get()->ReleaseScene(m_Scene);

		delete this;
	}

	void World::DestroyWorldActors()
	{
		for (auto it = m_Actors.begin(); it != m_Actors.end(); )
		{
			Actor* ToDelActor = *it;

			std::vector<Actor*> RemovedActorList;
			RemovedActorList.push_back(ToDelActor);
			OnRemoveActors.Braodcast( RemovedActorList );

			ToDelActor->UnRegisterComponents();
			it = m_Actors.erase(it);

			delete ToDelActor;
		}
		
		for (auto it = m_NewActors.begin(); it != m_NewActors.end(); )
		{
			Actor* ToDelActor = *it;

			std::vector<Actor*> RemovedActorList;
			RemovedActorList.push_back(ToDelActor);
			OnRemoveActors.Braodcast( RemovedActorList );

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