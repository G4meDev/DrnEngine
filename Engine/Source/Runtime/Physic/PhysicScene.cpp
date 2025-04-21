#include "DrnPCH.h"
#include "PhysicScene.h"
#include "PhysicManager.h"

LOG_DEFINE_CATEGORY( LogPhysicScene, "PhysicScene" )

namespace Drn
{
	PhysicScene::PhysicScene(World* InWorld)
		: m_OwningWorld(InWorld)
	{
		const physx::PxVec3 m_Gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );

		physx::PxSceneDesc sceneDesc(PhysicManager::Get()->GetToleranceScale());
		sceneDesc.gravity = m_Gravity;
		
		physx::PxU32 numWorkers = 8;
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate(numWorkers);
		sceneDesc.cpuDispatcher	= m_Dispatcher;
		sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;

		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		m_PhysxScene = PhysicManager::Get()->GetPhysics()->createScene(sceneDesc);

#if WITH_EDITOR

		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eSCALE, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_DYNAMIC, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_STATIC, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_SHAPES, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_EDGES, 1 );

#endif

#if WITH_PVD
		physx::PxPvdSceneClient* pvdClient = m_PhysxScene->getScenePvdClient();

		if ( pvdClient )
		{
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, false );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );
		}
#endif

		//AddTestActors();
	}

	PhysicScene::~PhysicScene()
	{
		PX_RELEASE(m_PhysxScene);
		PX_RELEASE(m_Dispatcher);
	}

	void PhysicScene::Release()
	{
		delete this;
	}

	void PhysicScene::Tick( float DeltaTime )
	{
		if (IsSimulating())
		{
			StepSimulation(DeltaTime);
			SyncActors();
		}

		if (m_DrawDebugCollision)
		{
			DrawDebugCollisions();
		}
	}

	bool PhysicScene::IsSimulating() const
	{
		return m_PhysxScene && m_OwningWorld && m_OwningWorld->IsTicking();
	}

	void PhysicScene::AddActor( physx::PxActor* InActor )
	{
		if (m_PhysxScene)
		{
			m_PhysxScene->addActor(*InActor);
		}

		else
		{
			LOG(LogPhysicScene, Error, "tring to add actor to non existing physic scene.")
		}
	}

	void PhysicScene::RemoveActor( physx::PxActor* InActor )
	{
		if (m_PhysxScene)
		{
			m_PhysxScene->removeActor(*InActor);
		}

		else
		{
			LOG(LogPhysicScene, Error, "tring to remove actor from non existing physic scene.")
		}
	}

	void PhysicScene::StepSimulation( float DeltaTime )
	{
		m_PhysxScene->lockWrite();

		m_PhysxScene->simulate(DeltaTime);
		m_PhysxScene->fetchResults(true);

		m_PhysxScene->unlockWrite();
	}

	void PhysicScene::SyncActors()
	{
		physx::PxU32 ActorCount = 0;
		physx::PxActor** ActiveActors = m_PhysxScene->getActiveActors(ActorCount);

		for (physx::PxU32 i = 0; i < ActorCount; i++)
		{
			physx::PxActor* ActiveActor = ActiveActors[i];
			physx::PxRigidActor* RigidActor = ActiveActor->is<physx::PxRigidActor>();

			if (BodyInstance* Body = PhysicUserData::Get<BodyInstance>(RigidActor->userData))
			{
				physx::PxTransform transform(RigidActor->getGlobalPose());
				Body->GetOwnerComponent()->SetWorldTransform(P2Transform(transform));
			}
		}
	}

	void PhysicScene::AddTestActors()
	{
		physx::PxPhysics* m_Physics = PhysicManager::Get()->GetPhysics();

		m_Material = m_Physics->createMaterial( 0.5f, 0.5f, 0.6f );
		physx::PxU32 size = 20;
		physx::PxTransform t(physx::PxVec3(0));

		physx::PxRigidStatic* GroundPlane = physx::PxCreatePlane(*m_Physics, physx::PxPlane(0, 1, 0, 1),
			*m_Material);
		m_PhysxScene->addActor(*GroundPlane);

		float halfExtent = 0.5f;
		physx::PxShape* shape = m_Physics->createShape( physx::PxBoxGeometry( halfExtent, halfExtent,
		halfExtent ), *m_Material );

		for ( physx::PxU32 i = 0; i < size; i++ )
		{
			for ( physx::PxU32 j = 0; j < size - i; j++ )
			{
				physx::PxTransform localTm( physx::PxVec3( physx::PxReal( j * 2 ) - physx::PxReal( size - i ), physx::PxReal( i * 2 + 1 ), 0 ) * halfExtent );
				physx::PxRigidDynamic* body = m_Physics->createRigidDynamic( t.transform( localTm ) );
				body->attachShape( *shape );
				
				physx::PxRigidBodyExt::updateMassAndInertia( *body, 10.0f );
				m_PhysxScene->addActor( *body );
			}
		}
		shape->release();
	}

// ------------------------------------------------------------------------------------------------------------

	void PhysicScene::DrawDebugCollisions()
	{
		PxActorTypeFlags Flags = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
	
		uint32 NumActors = GetPhysxScene()->getNbActors(Flags);
		PxActor** Actors = new PxActor*[NumActors];

		uint32 ActorsNum = GetPhysxScene()->getActors(Flags, Actors, NumActors);

		for (int32 i = 0; i < ActorsNum; i++)
		{
			PxActor* Actor = Actors[i];
				
			if ( Actor && Actor->getOwnerClient() == PX_DEFAULT_CLIENT )
			{
				PxRigidActor* RigidActor = Actor->is<PxRigidActor>();

				if (RigidActor)
				{
					Transform RigidTransform = P2Transform(RigidActor->getGlobalPose());

					Vector ActorLocation = RigidTransform.GetLocation();
					m_OwningWorld->DrawDebugLine(ActorLocation, ActorLocation + Vector::UpVector * 3, Vector::OneVector, 0);
				}
			}
		}

		delete Actors;
	}

}