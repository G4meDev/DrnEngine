#pragma once

#include "ForwardTypes.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

LOG_DECLARE_CATEGORY(LogPhysicManager)

namespace Drn
{
	class PxCustomErrorCallback : public PxErrorCallback
	{
	public:
		PxCustomErrorCallback() {};
		virtual	~PxCustomErrorCallback() {};

		virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override;
	};

	class PhysicManager
	{
	public:

		static void Init();
		static void Shutdown();

		void Tick(float DeltaTime);

		inline static PhysicManager* Get() { return m_SingletonInstance; }

		void InitalizePhysx();
		void ShutdownPhysx();

		PhysicScene* AllocateScene(World* InWorld);
		void RemoveAndInvalidateScene(PhysicScene*& InScene);

		inline physx::PxTolerancesScale GetToleranceScale() const { return m_Physics->getTolerancesScale(); };
		inline physx::PxPhysics* GetPhysics() { return m_Physics; }

	protected:

		static PhysicManager* m_SingletonInstance;

		//physx::PxDefaultErrorCallback m_DefaultErrorCallback;
		PxCustomErrorCallback m_DefaultErrorCallback;
		physx::PxDefaultAllocator m_DefaultAllocatorCallback;

		physx::PxFoundation* m_Foundation;
		physx::PxPhysics* m_Physics;

#if WITH_PVD
		physx::PxPvd* m_Pvd;
#endif

		std::set<PhysicScene*> m_AllocatedScenes;

	private:
	};
}