#pragma once

#include "ForwardTypes.h"

#include <PxConfig.h>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
//#include <pvd/PxPvdTransport.h>

LOG_DECLARE_CATEGORY(LogPhysicManager)

namespace Drn
{
	class PhysicManager
	{
	public:

		static void Init();
		static void Shutdown();

		void Tick(float DeltaTime);

		inline static PhysicManager* Get() { return m_SingletonInstance; }

		void InitalizePhysx();

	protected:

		static PhysicManager* m_SingletonInstance;

		physx::PxDefaultErrorCallback m_DefaultErrorCallback;
		physx::PxDefaultAllocator m_DefaultAllocatorCallback;

		physx::PxFoundation* m_Foundation;
		physx::PxPvd* m_Pvd;
		physx::PxPhysics* m_Physics;
	
		physx::PxDefaultCpuDispatcher* m_Dispatcher;

		physx::PxScene* m_Scene;
		
		physx::PxMaterial* m_Material;


		const physx::PxVec3 m_Gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );

	private:
	};
}