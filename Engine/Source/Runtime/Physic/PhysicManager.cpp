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
			m_SingletonInstance->ShutdownPhysx();

			delete m_SingletonInstance;
			m_SingletonInstance = nullptr;
		}
	}


	void PhysicManager::Tick( float DeltaTime )
	{
		SCOPE_STAT();

		for (PhysicScene* S : m_AllocatedScenes)
		{
			S->Tick(DeltaTime);
		}
	}

	void PhysicManager::InitalizePhysx()
	{
		m_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, m_DefaultAllocatorCallback, m_DefaultErrorCallback );
		if (!m_Foundation)
		{
			LOG(LogPhysicManager, Error, "PxCreateFoundation failed!")
		}

#if WITH_PVD
		m_Pvd = PxCreatePvd( *m_Foundation );

		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate( "127.0.0.1", 5425, 10 );
		m_Pvd->connect( *transport, physx::PxPvdInstrumentationFlag::eALL );
		m_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), true, m_Pvd );
#else
		m_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), true, nullptr );
#endif
	}

	void PhysicManager::ShutdownPhysx()
	{
		PX_RELEASE(m_Physics);

#if WITH_PVD
		if (m_Pvd)
		{
			physx::PxPvdTransport* transport = m_Pvd->getTransport();

			PX_RELEASE(m_Pvd);
			PX_RELEASE(transport);
		}
#endif

		PX_RELEASE(m_Foundation);
	}

	PhysicScene* PhysicManager::AllocateScene( World* InWorld )
	{
		PhysicScene* NewWorld = new PhysicScene(InWorld);
		m_AllocatedScenes.insert(NewWorld);

		return NewWorld;
	}

	void PhysicManager::RemoveAndInvalidateScene( PhysicScene*& InScene )
	{
		m_AllocatedScenes.erase(InScene);

		InScene->Release();
		InScene = nullptr;
	}

}