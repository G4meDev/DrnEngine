#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY( LogPhysicScene )

namespace Drn
{
	class PhysicScene
	{
	public:

		PhysicScene(World* InWorld);
		~PhysicScene();

		void Release();

		inline World* GetOwningWorld() { return m_OwningWorld; }
		inline physx::PxScene* GetPhysxScene() { return m_PhysxScene; }

		virtual void Tick(float DeltaTime);

		inline bool IsSimulating() const;

		void AddActor(physx::PxActor* InActor);
		void RemoveActor(physx::PxActor* InActor);

		//TODO: add force, clear velocity, set target, add actor, ...

	private:

		void StepSimulation(float DeltaTime);

		void SyncActors();

		void AddTestActors();

		World* m_OwningWorld;
		physx::PxScene* m_PhysxScene;

		physx::PxDefaultCpuDispatcher* m_Dispatcher;

		physx::PxMaterial* m_Material;
	};
}