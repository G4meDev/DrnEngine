#include "DrnPCH.h"
#include "PhysicManager.h"

LOG_DEFINE_CATEGORY( LogPhysicManager, "PhysicManager" )

namespace Drn
{
	PhysicManager* PhysicManager::m_SingletonInstance;

	void PhysicManager::Init()
	{
		LOG(LogPhysicManager, Info, "Initializing physic manager.")

		m_SingletonInstance = new PhysicManager();
		m_SingletonInstance->InitalizePhysx();
	}

	void PhysicManager::Shutdown()
	{
		if (m_SingletonInstance)
		{
			delete m_SingletonInstance;
			m_SingletonInstance = nullptr;
		}
	}


	void PhysicManager::Tick( float DeltaTime )
	{
		m_Scene->simulate(DeltaTime);
		m_Scene->fetchResults(true);
	}

	void PhysicManager::InitalizePhysx()
	{
		m_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, m_DefaultAllocatorCallback, m_DefaultErrorCallback );
		if (!m_Foundation)
		{
			LOG(LogPhysicManager, Error, "PxCreateFoundation failed!")
		}

		m_Pvd = PxCreatePvd( *m_Foundation );

		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate( "127.0.0.1", 5425, 10 );
		m_Pvd->connect( *transport, physx::PxPvdInstrumentationFlag::eALL );
		m_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), true, m_Pvd );


		physx::PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
		sceneDesc.gravity = m_Gravity;
		
		physx::PxU32 numWorkers = 1;
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate(numWorkers);
		sceneDesc.cpuDispatcher	= m_Dispatcher;
		sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;

		m_Scene = m_Physics->createScene(sceneDesc);
		physx::PxPvdSceneClient* pvdClient = m_Scene->getScenePvdClient();

		if ( pvdClient )
		{
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, false );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );
		}

// ------------------------------------------------------------------------------------------------------------------------------------

		m_Material = m_Physics->createMaterial( 0.5f, 0.5f, 0.6f );
		physx::PxU32 size = 20;
		physx::PxTransform t(physx::PxVec3(0));

		physx::PxRigidStatic* GroundPlane = physx::PxCreatePlane(*m_Physics, physx::PxPlane(0, 1, 0, 1), *m_Material);
		m_Scene->addActor(*GroundPlane);
		
		float halfExtent = 0.5f;
		physx::PxShape* shape = m_Physics->createShape( physx::PxBoxGeometry( halfExtent, halfExtent, halfExtent ), *m_Material );
		
		for ( physx::PxU32 i = 0; i < size; i++ )
		{
			for ( physx::PxU32 j = 0; j < size - i; j++ )
			{
				physx::PxTransform localTm( physx::PxVec3( physx::PxReal( j * 2 ) - physx::PxReal( size - i ), physx::PxReal( i * 2 + 1 ), 0 ) * halfExtent );
				physx::PxRigidDynamic* body = m_Physics->createRigidDynamic( t.transform( localTm ) );
				body->attachShape( *shape );
				physx::PxRigidBodyExt::updateMassAndInertia( *body, 10.0f );
				m_Scene->addActor( *body );
			}
		}
		shape->release();



	}

}