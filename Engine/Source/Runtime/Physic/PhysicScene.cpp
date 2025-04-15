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

		m_PhysxScene = PhysicManager::Get()->GetPhysics()->createScene(sceneDesc);

#if WITH_PVD
		physx::PxPvdSceneClient* pvdClient = m_PhysxScene->getScenePvdClient();

		if ( pvdClient )
		{
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, false );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );
		}
#endif

		AddTestActors();
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
		m_PhysxScene->simulate(DeltaTime);
		m_PhysxScene->fetchResults(true);
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

}